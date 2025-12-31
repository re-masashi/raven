use bevy::{
    asset::RenderAssetUsages,
    camera::CameraProjection,
    math::{bounding::Aabb3d, Vec3A},
    mesh::{Indices, Mesh},
    prelude::*,
    render::render_resource::PrimitiveTopology,
};
use noise::{Fbm, MultiFractal, NoiseFn, Perlin};
use std::collections::HashMap;

pub mod prelude {
    pub use crate::*;
}

const CHUNK_SIZE: usize = 128;
const CHUNK_SCALE: f32 = 2.0;
const HEIGHT_SCALE: f32 = 1.0;
const RENDER_DISTANCE: i32 = 4;

#[derive(Resource)]
pub struct TerrainConfig {
    pub chunk_size: usize,
    pub chunk_scale: f32,
    pub height_scale: f32,
    pub render_distance: i32,
    pub seed: u32,
}

impl Default for TerrainConfig {
    fn default() -> Self {
        Self {
            chunk_size: CHUNK_SIZE,
            chunk_scale: CHUNK_SCALE,
            height_scale: HEIGHT_SCALE,
            render_distance: RENDER_DISTANCE,
            seed: 42,
        }
    }
}

#[derive(Resource)]
struct TerrainGenerator {
    base_terrain: Fbm<Perlin>,
    mountain_noise: Fbm<Perlin>,
    detail_noise: Fbm<Perlin>,
    moisture: Fbm<Perlin>,
    temperature: Perlin,
}

impl TerrainGenerator {
    fn new(_seed: u32) -> Self {
        let fixed_seed = 12345;
        Self {
            base_terrain: Fbm::new(fixed_seed)
                .set_octaves(6)
                .set_lacunarity(2.0)
                .set_persistence(0.5),
            mountain_noise: Fbm::new(fixed_seed.wrapping_add(1))
                .set_octaves(4)
                .set_lacunarity(2.2)
                .set_persistence(0.6),
            detail_noise: Fbm::new(fixed_seed.wrapping_add(2))
                .set_octaves(2)
                .set_lacunarity(2.0)
                .set_persistence(0.25),
            moisture: Fbm::new(fixed_seed.wrapping_add(3))
                .set_octaves(4)
                .set_lacunarity(2.0)
                .set_persistence(0.5),
            temperature: Perlin::new(fixed_seed.wrapping_add(4)),
        }
    }

    fn get_height(&self, x: f64, z: f64) -> f32 {
        let scale = 0.002;

        let base = self.base_terrain.get([x * scale, z * scale]) as f32;
        let mountain_raw = self.mountain_noise.get([x * scale * 0.6, z * scale * 0.6]) as f32;
        let mountain = smooth_curve(((mountain_raw + 1.0) * 0.5).clamp(0.0, 0.8));
        let detail = self.detail_noise.get([x * scale * 4.0, z * scale * 4.0]) as f32 * 0.01;

        let base_height = base * 70.0;
        let mountain_height = mountain * 75.0;

        let height = base_height.lerp(mountain_height, mountain * 0.25);

        let result = height + detail + 8.0;
        result.clamp(-3.0, 80.0)
    }

    fn get_moisture(&self, x: f64, z: f64) -> f32 {
        let raw = self.moisture.get([x * 0.0008, z * 0.0008]) as f32;
        (raw * 0.5 + 0.5).clamp(0.0, 1.0)
    }

    fn get_temperature(&self, x: f64, z: f64, height: f32) -> f32 {
        let raw = self.temperature.get([x * 0.0008, z * 0.0008]) as f32;
        let base_temp = (raw * 0.5 + 0.5).clamp(0.0, 1.0);
        (base_temp - (height / 450.0)).clamp(0.0, 1.0)
    }
}

fn smooth_curve(x: f32) -> f32 {
    x * x * (3.0 - 2.0 * x)
}

#[derive(Component, Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub struct ChunkPosition {
    pub x: i32,
    pub z: i32,
}

#[derive(Component)]
struct TerrainChunk;

#[derive(Component, Clone, Copy, PartialEq, Eq, Debug)]
enum ChunkLOD {
    High,
    Medium,
    Low,
}

#[derive(Resource)]
struct LoadedChunks(HashMap<(i32, i32), Entity>);

pub struct TerrainPlugin;

impl Plugin for TerrainPlugin {
    fn build(&self, app: &mut App) {
        app.init_resource::<TerrainConfig>()
            .insert_resource(LoadedChunks(HashMap::new()))
            .insert_resource(TerrainGenerator::new(rand::random::<u32>()))
            .add_systems(Startup, setup_terrain)
            .add_systems(
                Update,
                (update_terrain, frustum_cull_chunks, update_chunk_lod),
            );
    }
}

fn setup_terrain(
    mut commands: Commands,
    config: Res<TerrainConfig>,
    mut loaded_chunks: ResMut<LoadedChunks>,
    generator: Res<TerrainGenerator>,
    mut meshes: ResMut<Assets<Mesh>>,
    mut materials: ResMut<Assets<StandardMaterial>>,
) {
    let material = materials.add(StandardMaterial {
        unlit: false,
        perceptual_roughness: 0.9,
        metallic: 0.0,
        ..default()
    });

    for x in -config.render_distance..=config.render_distance {
        for z in -config.render_distance..=config.render_distance {
            spawn_chunk(
                &mut commands,
                &config,
                &generator,
                &mut meshes,
                material.clone(),
                x,
                z,
                &mut loaded_chunks,
            );
        }
    }
}

fn update_terrain(
    mut commands: Commands,
    config: Res<TerrainConfig>,
    mut loaded_chunks: ResMut<LoadedChunks>,
    generator: Res<TerrainGenerator>,
    mut meshes: ResMut<Assets<Mesh>>,
    mut materials: ResMut<Assets<StandardMaterial>>,
    cameras: Query<&Transform, With<bevy::prelude::Camera>>,
) {
    let camera_transform = match cameras.single() {
        Ok(t) => t,
        Err(_) => return,
    };

    let camera_pos = camera_transform.translation;
    let current_chunk_x =
        (camera_pos.x / (config.chunk_size as f32 * config.chunk_scale)).floor() as i32;
    let current_chunk_z =
        (camera_pos.z / (config.chunk_size as f32 * config.chunk_scale)).floor() as i32;

    let mut chunks_to_remove = Vec::new();
    let mut chunks_to_add = Vec::new();

    for (&(chunk_x, chunk_z), &entity) in loaded_chunks.0.iter() {
        let dist_x = (chunk_x - current_chunk_x).abs();
        let dist_z = (chunk_z - current_chunk_z).abs();

        if dist_x > config.render_distance || dist_z > config.render_distance {
            chunks_to_remove.push(((chunk_x, chunk_z), entity));
        }
    }

    for x in -config.render_distance..=config.render_distance {
        for z in -config.render_distance..=config.render_distance {
            let world_chunk_x = current_chunk_x + x;
            let world_chunk_z = current_chunk_z + z;

            if !loaded_chunks
                .0
                .contains_key(&(world_chunk_x, world_chunk_z))
            {
                chunks_to_add.push((world_chunk_x, world_chunk_z));
            }
        }
    }

    let keys_to_remove: Vec<(i32, i32)> = chunks_to_remove.iter().map(|(key, _)| *key).collect();

    for (_, entity) in &chunks_to_remove {
        commands.entity(*entity).despawn();
    }

    let material = materials.add(StandardMaterial {
        unlit: true,
        ..default()
    });

    for (x, z) in chunks_to_add {
        spawn_chunk(
            &mut commands,
            &config,
            &generator,
            &mut meshes,
            material.clone(),
            x,
            z,
            &mut loaded_chunks,
        );
    }

    for key in keys_to_remove {
        loaded_chunks.0.remove(&key);
    }
}

fn spawn_chunk(
    commands: &mut Commands,
    config: &TerrainConfig,
    generator: &TerrainGenerator,
    meshes: &mut Assets<Mesh>,
    material: Handle<StandardMaterial>,
    chunk_x: i32,
    chunk_z: i32,
    loaded_chunks: &mut LoadedChunks,
) {
    let mesh = generate_chunk_mesh(config, generator, chunk_x, chunk_z, ChunkLOD::High);
    let mesh_handle = meshes.add(mesh);

    let world_x = chunk_x as f32 * config.chunk_size as f32 * config.chunk_scale;
    let world_z = chunk_z as f32 * config.chunk_size as f32 * config.chunk_scale;

    let entity = commands
        .spawn((
            Mesh3d(mesh_handle),
            MeshMaterial3d(material),
            Transform::from_xyz(world_x, 0.0, world_z),
            ChunkPosition {
                x: chunk_x,
                z: chunk_z,
            },
            TerrainChunk,
            ChunkLOD::High,
        ))
        .id();

    loaded_chunks.0.insert((chunk_x, chunk_z), entity);
}

fn generate_chunk_mesh(
    config: &TerrainConfig,
    generator: &TerrainGenerator,
    chunk_x: i32,
    chunk_z: i32,
    lod: ChunkLOD,
) -> Mesh {
    let step = match lod {
        ChunkLOD::High => 1,
        ChunkLOD::Medium => 2,
        ChunkLOD::Low => 4,
    };

    let size = config.chunk_size;
    let grid_size = size / step;

    let offset_x = chunk_x as f32 * size as f32 * config.chunk_scale;
    let offset_z = chunk_z as f32 * size as f32 * config.chunk_scale;

    let border = step;
    let expanded_size = size + border * 2;
    let expanded_grid_size = expanded_size / step;

    let mut all_positions = Vec::with_capacity((expanded_grid_size + 1) * (expanded_grid_size + 1));
    let mut all_heights = Vec::with_capacity((expanded_grid_size + 1) * (expanded_grid_size + 1));

    for z in (0..=expanded_size).step_by(step) {
        for x in (0..=expanded_size).step_by(step) {
            let world_x = offset_x + (x as f32 - border as f32) * config.chunk_scale;
            let world_z = offset_z + (z as f32 - border as f32) * config.chunk_scale;

            let height = generator.get_height(world_x as f64, world_z as f64);
            all_heights.push(height);
            all_positions.push([world_x - offset_x, height, world_z - offset_z]);
        }
    }

    let mut all_indices = Vec::new();
    for z in 0..expanded_grid_size {
        for x in 0..expanded_grid_size {
            let i = z * (expanded_grid_size + 1) + x;

            all_indices.push(i as u32);
            all_indices.push((i + expanded_grid_size + 2) as u32);
            all_indices.push((i + 1) as u32);

            all_indices.push(i as u32);
            all_indices.push((i + expanded_grid_size + 1) as u32);
            all_indices.push((i + expanded_grid_size + 2) as u32);
        }
    }

    let mut all_normals = vec![[0.0f32; 3]; all_positions.len()];
    calculate_normals(&all_positions, &all_indices, &mut all_normals);

    let num_vertices = (grid_size + 1) * (grid_size + 1);
    let num_indices = grid_size * grid_size * 6;

    let mut positions = Vec::with_capacity(num_vertices);
    let mut normals = Vec::with_capacity(num_vertices);
    let mut uvs = Vec::with_capacity(num_vertices);
    let mut colors = Vec::with_capacity(num_vertices);
    let mut indices = Vec::with_capacity(num_indices);

    for z in (0..=size).step_by(step) {
        for x in (0..=size).step_by(step) {
            let expanded_z = (z + border) / step;
            let expanded_x = (x + border) / step;
            let expanded_idx = expanded_z * (expanded_grid_size + 1) + expanded_x;

            let pos = all_positions[expanded_idx];
            positions.push(pos);

            let normal = all_normals[expanded_idx];
            normals.push(normal);

            uvs.push([x as f32 / size as f32, z as f32 / size as f32]);

            let world_x = offset_x + x as f32 * config.chunk_scale;
            let world_z = offset_z + z as f32 * config.chunk_scale;
            let height = pos[1];
            let moisture = generator.get_moisture(world_x as f64, world_z as f64);
            let temperature = generator.get_temperature(world_x as f64, world_z as f64, height);
            let color = get_terrain_color(height, moisture, temperature);
            colors.push([color.x, color.y, color.z, 1.0]);
        }
    }

    for z in 0..grid_size {
        for x in 0..grid_size {
            let i = z * (grid_size + 1) + x;

            indices.push(i as u32);
            indices.push((i + grid_size + 2) as u32);
            indices.push((i + 1) as u32);

            indices.push(i as u32);
            indices.push((i + grid_size + 1) as u32);
            indices.push((i + grid_size + 2) as u32);
        }
    }

    Mesh::new(
        PrimitiveTopology::TriangleList,
        RenderAssetUsages::MAIN_WORLD | RenderAssetUsages::RENDER_WORLD,
    )
    .with_inserted_attribute(Mesh::ATTRIBUTE_POSITION, positions)
    .with_inserted_attribute(Mesh::ATTRIBUTE_NORMAL, normals)
    .with_inserted_attribute(Mesh::ATTRIBUTE_UV_0, uvs)
    .with_inserted_attribute(Mesh::ATTRIBUTE_COLOR, colors)
    .with_inserted_indices(Indices::U32(indices))
}

fn calculate_normals(positions: &[[f32; 3]], indices: &[u32], normals: &mut [[f32; 3]]) {
    for i in (0..indices.len()).step_by(3) {
        let i0 = indices[i] as usize;
        let i1 = indices[i + 1] as usize;
        let i2 = indices[i + 2] as usize;

        let v0 = positions[i0];
        let v1 = positions[i1];
        let v2 = positions[i2];

        let edge1 = [v1[0] - v0[0], v1[1] - v0[1], v1[2] - v0[2]];
        let edge2 = [v2[0] - v0[0], v2[1] - v0[1], v2[2] - v0[2]];

        let mut normal = [
            edge1[1] * edge2[2] - edge1[2] * edge2[1],
            edge1[2] * edge2[0] - edge1[0] * edge2[2],
            edge1[0] * edge2[1] - edge1[1] * edge2[0],
        ];

        let length = (normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]).sqrt();
        if length > 0.0 {
            normal[0] /= length;
            normal[1] /= length;
            normal[2] /= length;
        }

        normals[i0][0] += normal[0];
        normals[i0][1] += normal[1];
        normals[i0][2] += normal[2];
        normals[i1][0] += normal[0];
        normals[i1][1] += normal[1];
        normals[i1][2] += normal[2];
        normals[i2][0] += normal[0];
        normals[i2][1] += normal[1];
        normals[i2][2] += normal[2];
    }

    for normal in normals.iter_mut() {
        let length = (normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]).sqrt();
        if length > 0.0 {
            normal[0] /= length;
            normal[1] /= length;
            normal[2] /= length;
        }
    }
}

#[allow(dead_code)]
fn get_rainbow_color(normalized_height: f32, time: f32) -> Vec3 {
    let hue = (normalized_height * 0.8 + time * 0.1) % 1.0;

    let h = hue * 6.0;
    let x = 1.0 - (h % 2.0 - 1.0).abs();

    let rgb = match h.floor() as i32 {
        0 => (1.0, x, 0.0),
        1 => (x, 1.0, 0.0),
        2 => (0.0, 1.0, x),
        3 => (0.0, x, 1.0),
        4 => (x, 0.0, 1.0),
        _ => (1.0, 0.0, x),
    };

    Vec3::new(rgb.0, rgb.1, rgb.2)
}

fn get_terrain_color(height: f32, moisture: f32, temperature: f32) -> Vec3 {
    if height < -2.0 {
        return Vec3::new(0.1, 0.3, 0.6);
    } else if height < 0.0 {
        return Vec3::new(0.15, 0.5, 0.7);
    }

    let base_color = if temperature > 0.7 && moisture < 0.4 {
        Vec3::new(0.76, 0.7, 0.5)
    } else if moisture > 0.6 && temperature > 0.4 {
        Vec3::new(0.2, 0.5, 0.2)
    } else if moisture > 0.5 && temperature < 0.4 {
        Vec3::new(0.3, 0.4, 0.25)
    } else if height > 30.0 {
        Vec3::new(0.55, 0.5, 0.45)
    } else if height > 15.0 {
        Vec3::new(0.4, 0.6, 0.35)
    } else if height > 5.0 {
        Vec3::new(0.45, 0.7, 0.3)
    } else {
        Vec3::new(0.5, 0.75, 0.35)
    };

    let snowy = Vec3::new(0.95, 0.95, 1.0);

    if height > 60.0 {
        let snow_factor = ((height - 60.0) / 40.0).clamp(0.0, 1.0);
        return base_color.lerp(snowy, snow_factor);
    }

    base_color
}

fn frustum_cull_chunks(
    config: Res<TerrainConfig>,
    mut terrain_chunks: Query<
        (&ChunkPosition, &mut ViewVisibility, &GlobalTransform),
        With<TerrainChunk>,
    >,
    cameras: Query<(&GlobalTransform, &Projection, &Camera)>,
) {
    let (camera_transform, projection, _camera) = match cameras.single() {
        Ok(c) => c,
        Err(_) => return,
    };

    let view_matrix = camera_transform.to_matrix().inverse();
    let proj_matrix = match projection {
        Projection::Perspective(p) => p.get_clip_from_view(),
        Projection::Orthographic(o) => o.get_clip_from_view(),
        Projection::Custom(_) => return,
    };
    let view_proj = proj_matrix * view_matrix;

    let frustum_planes = compute_frustum_planes(view_proj);

    for (_chunk_pos, mut view_visibility, global_transform) in terrain_chunks.iter_mut() {
        let chunk_world_size = config.chunk_size as f32 * config.chunk_scale;

        let center = global_transform.translation()
            + Vec3::new(chunk_world_size / 2.0, 40.0, chunk_world_size / 2.0);
        let half_extents = Vec3::new(chunk_world_size / 2.0, 40.0, chunk_world_size / 2.0);

        let aabb = Aabb3d::new(Vec3A::from(center), Vec3A::from(half_extents));

        let min: Vec3 = aabb.min.into();
        let max: Vec3 = aabb.max.into();

        if aabb_in_frustum(&min, &max, &frustum_planes) {
            *view_visibility = ViewVisibility::default();
        } else {
            *view_visibility = ViewVisibility::HIDDEN;
        }
    }
}

fn update_chunk_lod(
    config: Res<TerrainConfig>,
    generator: Res<TerrainGenerator>,
    mut meshes: ResMut<Assets<Mesh>>,
    mut terrain_chunks: Query<
        (&ChunkPosition, &mut ChunkLOD, &GlobalTransform, &mut Mesh3d),
        With<TerrainChunk>,
    >,
    cameras: Query<&GlobalTransform, With<bevy::prelude::Camera>>,
) {
    let camera_transform = match cameras.single() {
        Ok(t) => t,
        Err(_) => return,
    };

    let camera_pos = camera_transform.translation();

    for (chunk_pos, mut lod, global_transform, mut mesh_handle) in terrain_chunks.iter_mut() {
        let chunk_world_size = config.chunk_size as f32 * config.chunk_scale;
        let chunk_center = global_transform.translation()
            + Vec3::new(chunk_world_size / 2.0, 0.0, chunk_world_size / 2.0);

        let distance = camera_pos.distance(chunk_center) / chunk_world_size;

        let new_lod = match (distance, *lod) {
            (d, ChunkLOD::High) if d > 2.0 => ChunkLOD::Medium,
            (d, ChunkLOD::Medium) if d < 1.0 => ChunkLOD::High,
            (d, ChunkLOD::Medium) if d > 4.0 => ChunkLOD::Low,
            (d, ChunkLOD::Low) if d < 2.5 => ChunkLOD::Medium,
            _ => *lod,
        };

        if *lod != new_lod {
            let new_mesh =
                generate_chunk_mesh(&config, &generator, chunk_pos.x, chunk_pos.z, new_lod);
            *mesh_handle = Mesh3d(meshes.add(new_mesh));
            *lod = new_lod;
        }
    }
}

fn compute_frustum_planes(view_proj: Mat4) -> [Vec4; 6] {
    let m = view_proj;

    let r0 = Vec4::new(m.x_axis.x, m.y_axis.x, m.z_axis.x, m.w_axis.x);
    let r1 = Vec4::new(m.x_axis.y, m.y_axis.y, m.z_axis.y, m.w_axis.y);
    let r2 = Vec4::new(m.x_axis.z, m.y_axis.z, m.z_axis.z, m.w_axis.z);
    let r3 = Vec4::new(m.x_axis.w, m.y_axis.w, m.z_axis.w, m.w_axis.w);

    let left = r3 + r0;
    let right = r3 - r0;
    let bottom = r3 + r1;
    let top = r3 - r1;
    let near = r3 + r2;
    let far = r3 - r2;

    [left, right, bottom, top, near, far]
}

fn aabb_in_frustum(min: &Vec3, max: &Vec3, planes: &[Vec4; 6]) -> bool {
    for plane in planes {
        let normal = Vec3::new(plane.x, plane.y, plane.z);
        let len_sq = normal.length_squared();

        if len_sq > 1e-6 {
            let inv_len = 1.0 / len_sq.sqrt();
            let normal = normal * inv_len;

            let n_vertex = Vec3::new(
                if normal.x < 0.0 { max.x } else { min.x },
                if normal.y < 0.0 { max.y } else { min.y },
                if normal.z < 0.0 { max.z } else { min.z },
            );

            let signed_distance = normal.dot(n_vertex) + (plane.w * inv_len);

            if signed_distance < 0.0 {
                return false;
            }
        }
    }
    true
}

// #[allow(dead_code)]
// fn update_intoxicated_rainbow_colors(
//     _rainbow_time: Res<RainbowTime>,
//     config: Res<TerrainConfig>,
//     generator: Res<TerrainGenerator>,
//     terrain_chunks: Query<(&ChunkPosition, &Mesh3d), With<TerrainChunk>>,
//     mut meshes: ResMut<Assets<Mesh>>,
// ) {
//     for (chunk_pos, mesh_handle) in terrain_chunks.iter() {
//         if let Some(mesh) = meshes.get_mut(&mesh_handle.0) {
//             if let Some(positions) = mesh.attribute(Mesh::ATTRIBUTE_POSITION) {
//                 if let Some(positions_float3) = positions.as_float3() {
//                     let mut colors = Vec::new();

//                     let size = config.chunk_size;
//                     let offset_x = chunk_pos.x as f32 * size as f32 * config.chunk_scale;
//                     let offset_z = chunk_pos.z as f32 * size as f32 * config.chunk_scale;

//                     for (_i, pos) in positions_float3.iter().enumerate() {
//                         let world_x = offset_x + pos[0];
//                         let world_z = offset_z + pos[2];
//                         let height = pos[1];

//                         let moisture = generator.get_moisture(world_x as f64, world_z as f64);
//                         let temperature =
//                             generator.get_temperature(world_x as f64, world_z as f64, height);
//                         let color = get_terrain_color(height, moisture, temperature);
//                         colors.push([color.x, color.y, color.z, 1.0]);
//                     }

//                     mesh.insert_attribute(Mesh::ATTRIBUTE_COLOR, colors);
//                 }
//             }
//         }
//     }
// }
