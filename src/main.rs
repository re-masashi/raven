use bevy::diagnostic::{DiagnosticsStore, FrameTimeDiagnosticsPlugin};
use bevy::prelude::*;
use bevy_rapier3d::prelude::*;
use raven::flycam::*;
use raven::terrain::*;
use tracing_subscriber::prelude::*;

fn main() {
    let subscriber = tracing_subscriber::Registry::default().with(tracing_tracy::TracyLayer::new(
        tracing_tracy::DefaultConfig::default(),
    ));

    tracing::subscriber::set_global_default(subscriber)
        .expect("set global tracing subscriber failed");

    App::new()
        .add_plugins(DefaultPlugins.set(bevy::log::LogPlugin {
            filter: "raven=info,warn".into(),
            ..default()
        }))
        .add_plugins(bevy::diagnostic::FrameTimeDiagnosticsPlugin::default())
        .add_plugins(RapierPhysicsPlugin::<NoUserData>::default())
        .add_plugins(PlayerPlugin)
        .add_plugins(TerrainPlugin)
        .insert_resource(MovementSettings {
            sensitivity: 0.00015,
            speed: 12.0,
            sprint_speed: 24.0,
            ..default()
        })
        .add_systems(Startup, (setup, setup_fps_counter, setup_water))
        .add_systems(Update, (update_fps_counter, check_water_status))
        .insert_resource(ClearColor(Color::srgb(0.3, 0.6, 0.75)))
        .run();
}

fn setup(mut commands: Commands) {
    commands.spawn((
        DirectionalLight {
            illuminance: 5000.0,
            ..default()
        },
        Transform::from_rotation(Quat::from_euler(EulerRot::XYZ, -0.3, 0.5, 0.0)),
    ));

    commands.spawn((PointLight::default(), Transform::from_xyz(4.0, 20.0, 4.0)));

    info!("Move camera around by using WASD");
    info!("Hold Shift to sprint");
    info!("Use Spacebar to jump");
    info!("Use the mouse to look around");
    info!("Press Esc to hide or show the mouse cursor");
}

#[derive(Component)]
struct FpsText;

#[derive(Component)]
struct WaterStatusText;

#[derive(Component)]
struct WaterPlane;

fn setup_fps_counter(mut commands: Commands) {
    commands.spawn((
        Node {
            position_type: PositionType::Absolute,
            top: Val::Px(10.0),
            left: Val::Px(10.0),
            ..default()
        },
        Text::new("FPS: 0"),
        TextFont {
            font_size: 20.0,
            ..default()
        },
        TextColor(Color::WHITE),
        FpsText,
    ));
}

fn setup_water(
    mut commands: Commands,
    mut materials: ResMut<Assets<StandardMaterial>>,
    mut meshes: ResMut<Assets<Mesh>>,
) {
    let water_material = materials.add(StandardMaterial {
        base_color: Color::srgba(0.1, 0.4, 0.7, 0.5),
        unlit: false,
        perceptual_roughness: 0.1,
        metallic: 0.1,
        alpha_mode: AlphaMode::Blend,
        ..default()
    });

    commands.spawn((
        Mesh3d(meshes.add(Mesh::from(Plane3d::new(
            Vec3::Y,
            Vec2::new(10000.0, 10000.0),
        )))),
        MeshMaterial3d(water_material),
        Transform::from_xyz(0.0, 0.0, 0.0),
        WaterPlane,
    ));
}

fn check_water_status(
    mut commands: Commands,
    player_query: Query<&Transform, With<FlyCam>>,
    mut water_status_query: Query<&mut Text, With<WaterStatusText>>,
) {
    let in_water = match player_query.single() {
        Ok(transform) => transform.translation.y < 0.0,
        Err(_) => return,
    };

    if let Ok(mut text) = water_status_query.single_mut() {
        if in_water {
            text.0 = "IN WATER".to_string();
        } else {
            text.0 = String::new();
        }
    } else if in_water {
        commands.spawn((
            Node {
                position_type: PositionType::Absolute,
                top: Val::Px(50.0),
                left: Val::Px(10.0),
                ..default()
            },
            Text::new("IN WATER"),
            TextFont {
                font_size: 24.0,
                ..default()
            },
            TextColor(Color::srgb(0.5, 0.8, 1.0)),
            WaterStatusText,
        ));
    }
}

fn update_fps_counter(
    time: Res<Time>,
    mut timer: Local<Timer>,
    diagnostics: Res<DiagnosticsStore>,
    mut query: Query<&mut Text, With<FpsText>>,
) {
    if timer.duration().is_zero() {
        *timer = Timer::from_seconds(0.5, TimerMode::Repeating);
    }
    timer.tick(time.delta());
    if timer.just_finished() {
        for mut text in &mut query {
            if let Some(fps_diagnostic) = diagnostics.get(&FrameTimeDiagnosticsPlugin::FPS) {
                if let Some(value) = fps_diagnostic.smoothed() {
                    text.0 = format!("FPS: {:.1}", value);
                }
            }
        }
    }
}
