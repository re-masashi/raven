#import bevy_pbr::mesh_view_bindings
#import bevy_pbr::mesh_functions
#import bevy_pbr::mesh_vertex_bindings
#import bevy_pbr::pbr_types

struct RainbowMaterial {
    base_color: vec4<f32>,
    @location(0) height: f32,
}

@group(1) @binding(0)
var<uniform> material: RainbowMaterial;

struct VertexOutput {
    @builtin(position) clip_position: vec4<f32>,
    @location(0) world_position: vec4<f32>,
    @location(1) world_normal: vec3<f32>,
    @location(2) uv: vec2<f32>,
    @location(3) vertex_height: f32,
}

@vertex
fn vertex(
    @location(0) position: vec3<f32>,
    @location(1) normal: vec3<f32>,
    @location(2) uv: vec2<f32>,
) -> VertexOutput {
    var out: VertexOutput;
    
    let model = mesh.model;
    let world_position = mesh.get_world_position(position);
    
    out.clip_position = mesh_view.get_clip_position(world_position);
    out.world_position = world_position;
    out.world_normal = mesh.get_world_normal(normal);
    out.uv = uv;
    out.vertex_height = world_position.y;
    
    return out;
}

@fragment
fn fragment(in: VertexOutput) -> @location(0) vec4<f32> {
    let height = in.vertex_height;
    
    let red = sin(height * 0.1) * 0.5 + 0.5;
    let green = sin(height * 0.1 + 2.094) * 0.5 + 0.5;
    let blue = sin(height * 0.1 + 4.189) * 0.5 + 0.5;
    
    let rainbow_color = vec3<f32>(red, green, blue);
    
    let view_dir = normalize(mesh_view.world_position.xyz - in.world_position.xyz);
    let normal = normalize(in.world_normal);
    let light_dir = normalize(vec3<f32>(1.0, 1.0, 1.0));
    
    let diffuse = max(dot(normal, light_dir), 0.0);
    let ambient = 0.3;
    
    let final_color = rainbow_color * (diffuse + ambient);
    
    return vec4<f32>(final_color, 1.0);
}
