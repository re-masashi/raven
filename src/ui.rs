use bevy::prelude::*;
use bevy_lunex::prelude::*;

#[derive(Component)]
pub struct HomeScreen;

#[derive(Component)]
pub struct TitleText;

#[derive(Component)]
pub struct StartButton;

#[derive(Component)]
pub struct SubtitleText;

#[derive(States, Debug, Clone, PartialEq, Eq, Hash, Default)]
pub enum OnboardingState {
    #[default]
    HomeScreen,
    Onboarding,
    Playing,
}

#[derive(Resource, Default)]
pub struct OnboardingProgress {
    pub current_stage: usize,
    pub stage_timer: Timer,
}

#[derive(Component)]
pub struct OnboardingOverlay;

#[derive(Component)]
pub struct CinematicText {
    pub reveal_timer: Timer,
    pub text_content: String,
}

#[derive(Component)]
pub struct StageIndicator;

pub struct HomeScreenPlugin;

impl Plugin for HomeScreenPlugin {
    fn build(&self, app: &mut App) {
        app.add_plugins(UiLunexPlugins)
            .init_state::<OnboardingState>()
            .insert_resource(OnboardingProgress {
                current_stage: 0,
                stage_timer: Timer::from_seconds(4.0, TimerMode::Once),
            })
            .init_resource::<GameFade>()
            .add_systems(Startup, setup_home_screen)
            .add_systems(Update, handle_start_button)
            .add_systems(OnEnter(OnboardingState::Onboarding), hide_home_screen)
            .add_systems(OnEnter(OnboardingState::Onboarding), setup_onboarding)
            .add_systems(
                Update,
                run_onboarding_sequence.run_if(in_state(OnboardingState::Onboarding)),
            )
            .add_systems(
                Update,
                cinematic_text_reveal.run_if(in_state(OnboardingState::Onboarding)),
            )
            .add_systems(OnExit(OnboardingState::Onboarding), cleanup_onboarding)
            .add_systems(OnEnter(OnboardingState::Playing), spawn_game_fade)
            .add_systems(
                Update,
                update_game_fade.run_if(in_state(OnboardingState::Playing)),
            );
    }
}

fn hide_home_screen(mut query: Query<&mut Visibility, With<HomeScreen>>) {
    for mut visibility in query.iter_mut() {
        *visibility = Visibility::Hidden;
    }
}

fn show_home_screen(mut query: Query<&mut Visibility, With<HomeScreen>>) {
    for mut visibility in query.iter_mut() {
        *visibility = Visibility::Visible;
    }
}

#[derive(Component)]
pub struct GameFadeOverlay;

#[derive(Resource)]
pub struct GameFade {
    pub timer: Timer,
    pub is_fading_in: bool,
}

impl Default for GameFade {
    fn default() -> Self {
        Self {
            timer: Timer::from_seconds(2.0, TimerMode::Once),
            is_fading_in: true,
        }
    }
}

fn setup_home_screen(mut commands: Commands, _asset_server: Res<AssetServer>) {
    let mut root = commands.spawn((
        Name::new("Home Screen"),
        UiLayoutRoot::new_2d(),
        UiFetchFromCamera::<0>,
        HomeScreen,
    ));

    spawn_title(&mut root);
    spawn_subtitle(&mut root);
    spawn_start_button(&mut root);
    spawn_decorative_elements(&mut root);
    spawn_version_text(&mut root);
}

fn spawn_title(commands: &mut EntityCommands) {
    commands.with_children(|ui| {
        ui.spawn((
            Name::new("Title"),
            UiLayout::window()
                .pos(Rl((50.0, 30.0)))
                .size((800.0, 120.0))
                .pack(),
            UiColor::new(vec![
                (UiBase::id(), Color::srgba(1.0, 0.95, 0.8, 0.0)),
                (UiHover::id(), Color::srgba(1.0, 0.95, 0.8, 1.0)),
            ]),
            Text2d::new("RAVEN"),
            TextFont {
                font_size: 80.0,
                ..default()
            },
            TextColor(Color::srgb(1.0, 0.92, 0.8)),
            UiHover::new().forward_speed(1.5).backward_speed(0.8),
            TitleText,
        ))
        .observe(|_: On<Pointer<Over>>| {})
        .observe(|_: On<Pointer<Out>>| {});
    });
}

fn spawn_subtitle(commands: &mut EntityCommands) {
    commands.with_children(|ui| {
        ui.spawn((
            Name::new("Subtitle"),
            UiLayout::window()
                .pos(Rl((50.0, 42.0)))
                .size((600.0, 40.0))
                .pack(),
            UiColor::new(vec![
                (UiBase::id(), Color::srgba(0.9, 0.9, 1.0, 0.0)),
                (UiHover::id(), Color::srgba(0.9, 0.9, 1.0, 0.8)),
            ]),
            Text2d::new("A New World Awaits"),
            TextFont {
                font_size: 24.0,
                ..default()
            },
            TextColor(Color::srgb(0.9, 0.9, 1.0)),
            UiHover::new().forward_speed(1.0).backward_speed(0.5),
            SubtitleText,
        ))
        .observe(|_: On<Pointer<Over>>| {})
        .observe(|_: On<Pointer<Out>>| {});
    });
}

fn spawn_start_button(commands: &mut EntityCommands) {
    commands.with_children(|ui| {
        ui.spawn((
            Name::new("Start Button"),
            UiLayout::window()
                .pos(Rl((50.0, 65.0)))
                .size((300.0, 60.0))
                .pack(),
            UiColor::new(vec![
                (UiBase::id(), Color::srgba(0.95, 0.75, 0.5, 0.15)),
                (UiHover::id(), Color::srgba(1.0, 0.85, 0.65, 0.3)),
            ]),
            Sprite {
                color: Color::WHITE,
                custom_size: Some(Vec2::new(300.0, 60.0)),
                ..default()
            },
            UiHover::new().forward_speed(3.0).backward_speed(2.0),
            StartButton,
        ))
        .with_children(|button| {
            button.spawn((
                Name::new("Button Text"),
                UiLayout::window()
                    .pos(Rl((50.0, 50.0)))
                    .size((200.0, 30.0))
                    .pack(),
                UiColor::new(vec![
                    (UiBase::id(), Color::srgba(1.0, 0.92, 0.8, 1.0)),
                    (UiHover::id(), Color::srgb(1.0, 1.0, 1.0)),
                ]),
                Text2d::new("TAP TO START"),
                TextFont {
                    font_size: 32.0,
                    ..default()
                },
                TextColor(Color::srgb(1.0, 0.92, 0.8)),
                Pickable::IGNORE,
            ));
        })
        .observe(hover_set::<Pointer<Over>, true>)
        .observe(hover_set::<Pointer<Out>, false>)
        .observe(|_: On<Pointer<Click>>| {
            info!("Start button clicked!");
        });
    });
}

fn spawn_version_text(commands: &mut EntityCommands) {
    commands.with_children(|ui| {
        ui.spawn((
            Name::new("Version Text"),
            UiLayout::window()
                .pos(Rl((95.0, 95.0)))
                .size((200.0, 30.0))
                .pack(),
            UiColor::new(vec![(UiBase::id(), Color::srgba(1.0, 1.0, 1.0, 0.5))]),
            Text2d::new("v0.1.0"),
            TextFont {
                font_size: 16.0,
                ..default()
            },
            TextColor(Color::srgb(1.0, 1.0, 1.0)),
            Pickable::IGNORE,
        ));
    });
}

fn spawn_decorative_elements(commands: &mut EntityCommands) {
    commands.with_children(|ui| {
        ui.spawn((
            Name::new("Decorative Line 1"),
            UiLayout::window()
                .pos(Rl((50.0, 75.0)))
                .size((150.0, 2.0))
                .pack(),
            UiColor::new(vec![
                (UiBase::id(), Color::srgba(0.9, 0.8, 0.6, 0.0)),
                (UiHover::id(), Color::srgba(0.9, 0.8, 0.6, 0.6)),
            ]),
            Sprite {
                color: Color::WHITE,
                custom_size: Some(Vec2::new(150.0, 2.0)),
                ..default()
            },
            UiHover::new().forward_speed(0.8).backward_speed(0.4),
        ))
        .observe(|_: On<Pointer<Over>>| {})
        .observe(|_: On<Pointer<Out>>| {});

        ui.spawn((
            Name::new("Decorative Line 2"),
            UiLayout::window()
                .pos(Rl((50.0, 78.0)))
                .size((80.0, 2.0))
                .pack(),
            UiColor::new(vec![
                (UiBase::id(), Color::srgba(0.9, 0.8, 0.6, 0.0)),
                (UiHover::id(), Color::srgba(0.9, 0.8, 0.6, 0.4)),
            ]),
            Sprite {
                color: Color::WHITE,
                custom_size: Some(Vec2::new(80.0, 2.0)),
                ..default()
            },
            UiHover::new().forward_speed(0.8).backward_speed(0.4),
        ))
        .observe(|_: On<Pointer<Over>>| {})
        .observe(|_: On<Pointer<Out>>| {});
    });
}

fn handle_start_button(
    mut button_query: Query<&Interaction, (Changed<Interaction>, With<StartButton>)>,
    mut next_state: ResMut<NextState<OnboardingState>>,
) {
    for interaction in button_query.iter_mut() {
        if *interaction == Interaction::Pressed {
            info!("Starting onboarding...");
            next_state.set(OnboardingState::Onboarding);
        }
    }
}

fn setup_onboarding(mut commands: Commands) {
    let mut root = commands.spawn((
        Name::new("Onboarding Overlay"),
        UiLayoutRoot::new_2d(),
        UiFetchFromCamera::<0>,
        OnboardingOverlay,
    ));

    spawn_fade_overlay(&mut root);
    spawn_cinematic_text(&mut root, "In beginning...");
    spawn_stage_indicator(&mut root);
}

fn spawn_fade_overlay(commands: &mut EntityCommands) {
    commands.with_children(|ui| {
        ui.spawn((
            Name::new("Fade Overlay"),
            UiLayout::window()
                .pos(Rl((0.0, 0.0)))
                .size((100.0, 100.0))
                .pack(),
            UiColor::new(vec![(UiBase::id(), Color::srgba(0.0, 0.0, 0.05, 0.95))]),
            Sprite {
                color: Color::WHITE,
                custom_size: Some(Vec2::new(1920.0, 1080.0)),
                ..default()
            },
        ));
    });
}

fn spawn_cinematic_text(commands: &mut EntityCommands, text: &str) {
    commands.with_children(|ui| {
        ui.spawn((
            Name::new("Cinematic Text"),
            UiLayout::window()
                .pos(Rl((50.0, 50.0)))
                .size((1400.0, 100.0))
                .pack(),
            UiColor::new(vec![(UiBase::id(), Color::srgba(1.0, 1.0, 1.0, 0.0))]),
            Text2d::new(String::new()),
            TextFont {
                font_size: 48.0,
                ..default()
            },
            TextColor(Color::srgb(1.0, 1.0, 1.0)),
            CinematicText {
                reveal_timer: Timer::from_seconds(0.05, TimerMode::Repeating),
                text_content: text.to_string(),
            },
        ));
    });
}

fn spawn_stage_indicator(commands: &mut EntityCommands) {
    commands.with_children(|ui| {
        ui.spawn((
            Name::new("Stage Indicator"),
            UiLayout::window()
                .pos(Rl((50.0, 85.0)))
                .size((200.0, 40.0))
                .pack(),
            UiColor::new(vec![(UiBase::id(), Color::srgba(1.0, 1.0, 1.0, 0.6))]),
            Text2d::new("1 / 5"),
            TextFont {
                font_size: 20.0,
                ..default()
            },
            TextColor(Color::srgb(1.0, 1.0, 1.0)),
            StageIndicator,
        ));
    });
}

fn cinematic_text_reveal(time: Res<Time>, mut query: Query<(&mut CinematicText, &mut Text2d)>) {
    for (mut cinematic, mut text) in query.iter_mut() {
        if cinematic.text_content.len() > text.0.len() {
            cinematic.reveal_timer.tick(time.delta());
            if cinematic.reveal_timer.just_finished() {
                let current_len = text.0.len();
                let next_char = cinematic.text_content.chars().nth(current_len);
                if let Some(c) = next_char {
                    text.0.push(c);
                }
            }
        }
    }
}

fn run_onboarding_sequence(
    time: Res<Time>,
    mut progress: ResMut<OnboardingProgress>,
    mut cinematic_query: Query<&mut CinematicText>,
    mut stage_query: Query<&mut Text2d, With<StageIndicator>>,
    mut overlay_query: Query<&mut UiColor, With<OnboardingOverlay>>,
    mut next_state: ResMut<NextState<OnboardingState>>,
) {
    progress.stage_timer.tick(time.delta());

    const STAGES: &[&str] = &[
        "In beginning...",
        "The world was forged in silence and shadow.",
        "Ancient powers slumber beneath earth...",
        "Waiting for one who dares to awaken them.",
        "Your journey begins now.",
    ];

    if progress.stage_timer.just_finished() {
        progress.current_stage += 1;

        if progress.current_stage >= STAGES.len() {
            info!("Onboarding complete, starting game...");
            next_state.set(OnboardingState::Playing);
            return;
        }

        for mut cinematic in cinematic_query.iter_mut() {
            cinematic.text_content = STAGES[progress.current_stage].to_string();
        }

        for mut stage_text in stage_query.iter_mut() {
            stage_text.0 = format!("{} / {}", progress.current_stage + 1, STAGES.len());
        }

        progress.stage_timer.reset();

        if let Ok(mut color) = overlay_query.single_mut() {
            let new_alpha = match progress.current_stage {
                1 => 0.9,
                2 => 0.85,
                3 => 0.8,
                _ => 0.95,
            };
            if let Some(c) = color.get_mut(&UiBase::id()) {
                *c = Color::srgba(0.0, 0.0, 0.05, new_alpha);
            }
        }
    }
}

fn cleanup_onboarding(
    mut commands: Commands,
    overlay_query: Query<Entity, With<OnboardingOverlay>>,
) {
    for entity in overlay_query.iter() {
        commands.entity(entity).despawn();
    }
}

fn spawn_game_fade(mut commands: Commands, mut fade: ResMut<GameFade>) {
    fade.timer.reset();
    fade.is_fading_in = false;

    commands
        .spawn((
            Name::new("Game Fade Overlay"),
            UiLayoutRoot::new_2d(),
            UiFetchFromCamera::<0>,
            GameFadeOverlay,
        ))
        .with_children(|ui| {
            ui.spawn((
                Name::new("Fade Black"),
                UiLayout::window()
                    .pos(Rl((0.0, 0.0)))
                    .size((100.0, 100.0))
                    .pack(),
                UiColor::new(vec![(UiBase::id(), Color::srgba(0.0, 0.0, 0.0, 1.0))]),
                Sprite {
                    color: Color::WHITE,
                    custom_size: Some(Vec2::new(1920.0, 1080.0)),
                    ..default()
                },
            ));
        });
}

fn update_game_fade(
    time: Res<Time>,
    mut fade: ResMut<GameFade>,
    mut overlay_query: Query<&mut UiColor, With<GameFadeOverlay>>,
    mut commands: Commands,
    overlay_entity: Query<Entity, With<GameFadeOverlay>>,
) {
    fade.timer.tick(time.delta());

    let elapsed = fade.timer.elapsed_secs();
    let duration = fade.timer.duration().as_secs_f32();
    let progress = elapsed / duration;
    let alpha = 1.0 - progress.min(1.0);

    if let Ok(mut color) = overlay_query.single_mut() {
        if let Some(c) = color.get_mut(&UiBase::id()) {
            *c = Color::srgba(0.0, 0.0, 0.0, alpha);
        }
    }

    if fade.timer.is_finished() {
        if let Ok(entity) = overlay_entity.single() {
            commands.entity(entity).despawn();
        }
    }
}
