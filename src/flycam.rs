/// FPS camera with physics
use bevy::input::mouse::MouseMotion;
use bevy::prelude::*;
use bevy::window::{CursorGrabMode, CursorOptions, PrimaryWindow};
use bevy_rapier3d::prelude::*;

pub mod prelude {
    pub use crate::*;
}

/// Mouse sensitivity and movement speed
#[derive(Resource)]
pub struct MovementSettings {
    pub sensitivity: f32,
    pub speed: f32,
    pub jump_impulse: f32,
    pub air_acceleration: f32,
    pub gravity_scale: f32,
    pub fall_gravity_scale: f32,
    pub jump_hold_gravity_scale: f32,
    pub coyote_time: f32,
}

impl Default for MovementSettings {
    fn default() -> Self {
        Self {
            sensitivity: 0.00012,
            speed: 40.,
            jump_impulse: 8.0,
            air_acceleration: 15.0,
            gravity_scale: 1.0,
            fall_gravity_scale: 1.5,
            jump_hold_gravity_scale: 0.5,
            coyote_time: 0.15,
        }
    }
}

/// Key configuration
#[derive(Resource)]
pub struct KeyBindings {
    pub move_forward: KeyCode,
    pub move_backward: KeyCode,
    pub move_left: KeyCode,
    pub move_right: KeyCode,
    pub jump: KeyCode,
    pub toggle_grab_cursor: KeyCode,
}

impl Default for KeyBindings {
    fn default() -> Self {
        Self {
            move_forward: KeyCode::KeyW,
            move_backward: KeyCode::KeyS,
            move_left: KeyCode::KeyA,
            move_right: KeyCode::KeyD,
            jump: KeyCode::Space,
            toggle_grab_cursor: KeyCode::Escape,
        }
    }
}

/// Used in queries when you want flycams and not other cameras
/// A marker component used in queries when you want flycams and not other cameras
#[derive(Component)]
pub struct FlyCam;

#[derive(Component, Default)]
pub struct PlayerState {
    pub is_grounded: bool,
    pub time_since_grounded: f32,
    pub jump_hold_time: f32,
    pub has_jumped: bool,
}

/// Grabs/ungrabs mouse cursor
fn toggle_grab_cursor(mut primary_cursor_options: Single<&mut CursorOptions, With<PrimaryWindow>>) {
    match primary_cursor_options.grab_mode {
        CursorGrabMode::None => {
            primary_cursor_options.grab_mode = CursorGrabMode::Confined;
            primary_cursor_options.visible = false;
        }
        _ => {
            primary_cursor_options.grab_mode = CursorGrabMode::None;
            primary_cursor_options.visible = true;
        }
    }
}

/// Grabs the cursor when game first starts
fn initial_grab_cursor(primary_cursor_options: Single<&mut CursorOptions, With<PrimaryWindow>>) {
    toggle_grab_cursor(primary_cursor_options);
}

/// Spawns the `Camera3dBundle` to be controlled
fn setup_player(mut commands: Commands) {
    let spawn_y = 50.0;
    commands.spawn((
        Camera3d::default(),
        FlyCam,
        RigidBody::Dynamic,
        LockedAxes::ROTATION_LOCKED,
        Collider::capsule(Vec3::Y * -0.4, Vec3::Y * 0.4, 0.4),
        Friction::new(0.0),
        Restitution::new(0.0),
        GravityScale(1.0),
        Velocity::default(),
        AdditionalMassProperties::Mass(70.0),
        PlayerState::default(),
        Transform::from_xyz(0.0, spawn_y, 5.0),
    ));
}

/// Updates player state (grounded detection, coyote time, etc.)
fn player_state_update(time: Res<Time>, mut player_query: Query<(&mut PlayerState, &Velocity)>) {
    for (mut state, velocity) in player_query.iter_mut() {
        let was_grounded = state.is_grounded;

        if velocity.linvel.y.abs() < 0.1 && velocity.linvel.y >= -1.0 {
            state.is_grounded = true;
            state.time_since_grounded = 0.0;

            if !was_grounded && state.has_jumped {
                state.has_jumped = false;
            }
        } else {
            state.is_grounded = false;
            state.time_since_grounded += time.delta_secs();
        }
    }
}

/// Handles keyboard input and movement
fn player_move(
    keys: Res<ButtonInput<KeyCode>>,
    primary_cursor_options: Single<&mut CursorOptions, With<PrimaryWindow>>,
    settings: Res<MovementSettings>,
    key_bindings: Res<KeyBindings>,
    mut player_query: Query<(&mut Velocity, &PlayerState), With<RigidBody>>,
    camera_query: Query<&Transform, With<FlyCam>>,
) {
    let _span = tracing::span!(tracing::Level::INFO, "player_move").entered();

    let Ok(camera_transform) = camera_query.single() else {
        return;
    };

    let local_z = camera_transform.local_z();
    let forward = -Vec3::new(local_z.x, 0., local_z.z).normalize();
    let right = Vec3::new(local_z.z, 0., -local_z.x).normalize();

    let mut move_input = Vec3::ZERO;

    for key in keys.get_pressed() {
        match primary_cursor_options.grab_mode {
            CursorGrabMode::None => (),
            _ => {
                let key = *key;
                if key == key_bindings.move_forward {
                    move_input += forward;
                } else if key == key_bindings.move_backward {
                    move_input -= forward;
                } else if key == key_bindings.move_left {
                    move_input -= right;
                } else if key == key_bindings.move_right {
                    move_input += right;
                }
            }
        }
    }

    for (mut velocity, state) in player_query.iter_mut() {
        if move_input != Vec3::ZERO {
            let target_velocity = move_input.normalize()
                * if state.is_grounded {
                    settings.speed
                } else {
                    settings.air_acceleration
                };
            let accel = if state.is_grounded { 0.3 } else { 0.15 };
            velocity.linvel.x = velocity.linvel.x.lerp(target_velocity.x, accel);
            velocity.linvel.z = velocity.linvel.z.lerp(target_velocity.z, accel);
        } else {
            velocity.linvel.x *= 0.9;
            velocity.linvel.z *= 0.9;
        }
    }
}

/// Handles looking around if cursor is locked
fn player_look(
    settings: Res<MovementSettings>,
    primary_window: Query<&mut Window, With<PrimaryWindow>>,
    primary_cursor_options: Single<&mut CursorOptions, With<PrimaryWindow>>,
    mut state: MessageReader<MouseMotion>,
    mut query: Query<&mut Transform, With<FlyCam>>,
) {
    if let Ok(window) = primary_window.single() {
        for mut transform in query.iter_mut() {
            for ev in state.read() {
                let (mut yaw, mut pitch, _) = transform.rotation.to_euler(EulerRot::YXZ);
                match primary_cursor_options.grab_mode {
                    CursorGrabMode::None => (),
                    _ => {
                        let window_scale = window.height().min(window.width());
                        pitch -= (settings.sensitivity * ev.delta.y * window_scale).to_radians();
                        yaw -= (settings.sensitivity * ev.delta.x * window_scale).to_radians();
                    }
                }

                pitch = pitch.clamp(-1.54, 1.54);

                transform.rotation =
                    Quat::from_axis_angle(Vec3::Y, yaw) * Quat::from_axis_angle(Vec3::X, pitch);
            }
        }
    } else {
        warn!("Primary window not found for `player_look`!");
    }
}

/// Handles jumping
fn player_jump(
    keys: Res<ButtonInput<KeyCode>>,
    primary_cursor_options: Single<&mut CursorOptions, With<PrimaryWindow>>,
    settings: Res<MovementSettings>,
    key_bindings: Res<KeyBindings>,
    mut player_query: Query<(&mut Velocity, &mut PlayerState, &mut GravityScale), With<RigidBody>>,
) {
    if primary_cursor_options.grab_mode == CursorGrabMode::None {
        return;
    }

    let jump_pressed = keys.pressed(key_bindings.jump);
    let jump_just_pressed = keys.just_pressed(key_bindings.jump);

    for (mut velocity, mut state, mut gravity_scale) in player_query.iter_mut() {
        if jump_just_pressed
            && (state.is_grounded || state.time_since_grounded < settings.coyote_time)
            && !state.has_jumped
        {
            velocity.linvel.y = settings.jump_impulse;
            state.is_grounded = false;
            state.time_since_grounded = settings.coyote_time;
            state.has_jumped = true;
        }

        if !state.is_grounded {
            state.jump_hold_time += 1.0;
            if jump_pressed && state.jump_hold_time < 15.0 && velocity.linvel.y > 0.0 {
                gravity_scale.0 = settings.jump_hold_gravity_scale;
            } else if velocity.linvel.y < 0.0 {
                gravity_scale.0 = settings.fall_gravity_scale;
            } else {
                gravity_scale.0 = settings.gravity_scale;
            }
        } else {
            gravity_scale.0 = settings.gravity_scale;
            state.jump_hold_time = 0.0;
        }
    }
}

fn cursor_grab(
    keys: Res<ButtonInput<KeyCode>>,
    key_bindings: Res<KeyBindings>,
    primary_cursor_options: Single<&mut CursorOptions, With<PrimaryWindow>>,
) {
    if keys.just_pressed(key_bindings.toggle_grab_cursor) {
        toggle_grab_cursor(primary_cursor_options);
    }
}

// Grab cursor when an entity with FlyCam is added
fn initial_grab_on_flycam_spawn(
    query_added: Query<Entity, Added<FlyCam>>,
    primary_cursor_options: Single<&mut CursorOptions, With<PrimaryWindow>>,
) {
    if query_added.is_empty() {
        return;
    }

    toggle_grab_cursor(primary_cursor_options);
}

/// Contains everything needed to add first-person FPS camera behavior to your game
pub struct PlayerPlugin;
impl Plugin for PlayerPlugin {
    fn build(&self, app: &mut App) {
        app.init_resource::<MovementSettings>()
            .init_resource::<KeyBindings>()
            .add_systems(Startup, setup_player)
            .add_systems(Startup, initial_grab_cursor)
            .add_systems(Update, player_state_update)
            .add_systems(Update, player_move)
            .add_systems(Update, player_look)
            .add_systems(Update, player_jump)
            .add_systems(Update, cursor_grab);
    }
}

/// Same as [`PlayerPlugin`] but does not spawn a camera
pub struct NoCameraPlayerPlugin;
impl Plugin for NoCameraPlayerPlugin {
    fn build(&self, app: &mut App) {
        app.init_resource::<MovementSettings>()
            .init_resource::<KeyBindings>()
            .add_systems(Startup, initial_grab_cursor)
            .add_systems(Startup, initial_grab_on_flycam_spawn)
            .add_systems(Update, player_state_update)
            .add_systems(Update, player_move)
            .add_systems(Update, player_look)
            .add_systems(Update, player_jump)
            .add_systems(Update, cursor_grab);
    }
}
