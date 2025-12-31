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
            speed: 50.0,
        })
        .add_systems(Startup, (setup, setup_fps_counter))
        .add_systems(Update, update_fps_counter)
        .run();
}

fn setup(
    mut commands: Commands,
    mut meshes: ResMut<Assets<Mesh>>,
    mut materials: ResMut<Assets<StandardMaterial>>,
) {
    commands.spawn((
        Mesh3d(meshes.add(Cuboid::new(1.0, 1.0, 1.0))),
        MeshMaterial3d(materials.add(Color::srgb(0.8, 0.7, 0.6))),
        Transform::from_xyz(0.0, 10.0, 0.0),
        RigidBody::Dynamic,
        Collider::cuboid(0.5, 0.5, 0.5),
    ));

    commands.spawn((
        DirectionalLight {
            illuminance: 10000.0,
            ..default()
        },
        Transform::from_rotation(Quat::from_euler(EulerRot::XYZ, -0.3, 0.5, 0.0)),
    ));

    commands.spawn((PointLight::default(), Transform::from_xyz(4.0, 20.0, 4.0)));

    info!("Move camera around by using WASD for lateral movement");
    info!("Use Left Shift and Spacebar for vertical movement");
    info!("Use the mouse to look around");
    info!("Press Esc to hide or show the mouse cursor");
}

#[derive(Component)]
struct FpsText;

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
