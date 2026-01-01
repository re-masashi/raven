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
        .add_systems(Startup, (setup, setup_fps_counter))
        .add_systems(Update, update_fps_counter)
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
