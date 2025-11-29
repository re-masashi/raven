#ifndef DB_PERLIN_IMPL
#define DB_PERLIN_IMPL
#include "db_perlin.hpp"
#endif // !DEBUG

#include "game.h"
#include "raymath.h"
#include <algorithm>
#include <cmath>
#include <format>
#include <iostream>
#include <string>
#include <vector>

void InitGame(GameContext& ctx) {
  std::cout << "Game Initialized" << std::endl;

  ctx.font = LoadFontEx("/usr/share/fonts/TTF/Roboto-Medium.ttf", 20, nullptr, 0);

  ctx.lightingShader =
      LoadShader("src/shaders/vertex.glsl", "src/shaders/fragment.glsl");

  if (ctx.lightingShader.id == 0) {
    std::cout << "ERROR: Shader failed to load!" << std::endl;
  } else {
    std::cout << "Shader loaded successfully!" << std::endl;
  }

  GenerateStars(ctx);
  LoadVegetationModels(ctx);
  InitWater(ctx);

  Vector3 lightDir = {-0.4f, -0.8f, 0.4f};
  const float len = std::sqrt(lightDir.x * lightDir.x + lightDir.y * lightDir.y +
                              lightDir.z * lightDir.z);
  lightDir.x /= len;
  lightDir.y /= len;
  lightDir.z /= len;

  Vector3 lightColor = {1.0f, 0.85f, 1.0f};

  SetShaderValue(ctx.lightingShader, GetShaderLocation(ctx.lightingShader, "lightDir"),
                 &lightDir, SHADER_UNIFORM_VEC3);
  SetShaderValue(ctx.lightingShader,
                 GetShaderLocation(ctx.lightingShader, "lightColor"), &lightColor,
                 SHADER_UNIFORM_VEC3);
  initializeSpawnHut(ctx);

  ctx.camera.position = {ctx.spawnHut.position.x - 15.0f, ctx.spawnHut.position.y + 10.0f,
                     ctx.spawnHut.position.z - 15.0f};
  ctx.camera.target = ctx.spawnHut.position;
  ctx.camera.up = {0.0f, 1.0f, 0.0f};
  ctx.camera.fovy = 45.0f;
  ctx.camera.projection = CAMERA_PERSPECTIVE;

  for (int dx = -ctx.renderDistance; dx <= ctx.renderDistance; dx++) {
    for (int dz = -ctx.renderDistance; dz <= ctx.renderDistance; dz++) {
      generateChunk(ctx, dx, dz);
      GenerateVegetationForChunk(ctx, ctx.chunks[{dx, dz}]);
    }
  }
}

void UpdateGame(GameContext& ctx) {
  if (ctx.state == GameState::MENU) {
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();

    const Rectangle enterGameBtn = {static_cast<float>(screenWidth) / 2 - 150,
                                    static_cast<float>(screenHeight) / 2 - 50, 300, 50};
    const Rectangle settingsBtn = {static_cast<float>(screenWidth) / 2 - 150,
                                   static_cast<float>(screenHeight) / 2 + 10, 300, 50};

    if (CheckCollisionPointRec(GetMousePosition(), enterGameBtn) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      ctx.state = GameState::GAME;
      DisableCursor();
    }
    if (CheckCollisionPointRec(GetMousePosition(), settingsBtn) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      ctx.state = GameState::SETTINGS;
    }
    if (IsKeyPressed(KEY_G)) {
      ctx.state = GameState::GAME;
      DisableCursor();
    }
  } else if (ctx.state == GameState::GAME) {
    if (IsKeyPressed(KEY_ESCAPE)) {
      ctx.state = GameState::MENU;
      EnableCursor();
      return;
    }

    // Mouse look
    const Vector2 mouseDelta = GetMouseDelta();
    constexpr float sensitivity = 0.003f;
    ctx.cameraYaw -= mouseDelta.x * sensitivity;
    ctx.cameraPitch -= mouseDelta.y * sensitivity;

    constexpr float maxPitch = PI / 2.0f - 0.1f;
    ctx.cameraPitch = std::clamp(ctx.cameraPitch, -maxPitch, maxPitch);

    const float deltaTime = GetFrameTime();
    UpdatePlayer(ctx, deltaTime);

    // Teleport to spawn with H key
    if (IsKeyPressed(KEY_H)) {
      ctx.camera.position = {ctx.spawnHut.position.x - 15.0f, ctx.spawnHut.position.y + 10.0f,
                         ctx.spawnHut.position.z - 15.0f};
    }

    // Render distance
    if (IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_EQUAL)) {
      ctx.renderDistance = std::min(ctx.renderDistance + 1, 10);
    }
    if (IsKeyPressed(KEY_KP_SUBTRACT) || IsKeyPressed(KEY_MINUS)) {
      ctx.renderDistance = std::max(ctx.renderDistance - 1, 2);
    }

    SetShaderValue(ctx.lightingShader, GetShaderLocation(ctx.lightingShader, "viewPos"),
                   &ctx.camera.position, SHADER_UNIFORM_VEC3);

    // Chunk loading
    constexpr int stride = 31;
    const int cx = static_cast<int>(std::floor(ctx.camera.position.x / stride));
    const int cz = static_cast<int>(std::floor(ctx.camera.position.z / stride));

    for (int dx = -ctx.renderDistance; dx <= ctx.renderDistance; ++dx) {
      for (int dz = -ctx.renderDistance; dz <= ctx.renderDistance; ++dz) {
        const int ncx = cx + dx;
        const int ncz = cz + dz;

        if (!ctx.chunks.contains({ncx, ncz})) {
          generateChunk(ctx, ncx, ncz);
          GenerateVegetationForChunk(ctx, ctx.chunks[{ncx, ncz}]);
        }
      }
    }

    // Unload distant chunks
    const float unloadDistance = static_cast<float>((ctx.renderDistance + 2) * stride);
    std::vector<std::pair<int, int>> toUnload;

    for (const auto &[coords, chunk] : ctx.chunks) {
      const Vector3 chunkCenter = {static_cast<float>(chunk.x * stride + stride / 2), 15.0f,
                                   static_cast<float>(chunk.z * stride + stride / 2)};
      const float dist = Vector3Distance(ctx.camera.position, chunkCenter);

      if (dist > unloadDistance) {
        toUnload.emplace_back(coords.first, coords.second);
      }
    }

    for (const auto &key : toUnload) {
      UnloadModel(ctx.chunks[key].model);
      ctx.chunks.erase(key);
    }
  } else if (ctx.state == GameState::SETTINGS) {
    if (IsKeyPressed(KEY_ESCAPE)) {
      ctx.state = GameState::MENU;
    }
  }
}

void DrawGame(GameContext& ctx) {
  if (ctx.state == GameState::MENU) {
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();

    const Rectangle enterGameBtn = {static_cast<float>(screenWidth) / 2 - 150,
                                    static_cast<float>(screenHeight) / 2 - 50, 300, 50};
    const Rectangle settingsBtn = {static_cast<float>(screenWidth) / 2 - 150,
                                   static_cast<float>(screenHeight) / 2 + 10, 300, 50};

    const Color enterColor = CheckCollisionPointRec(GetMousePosition(), enterGameBtn)
                           ? Color{100, 100, 100, 255}
                           : Color{50, 50, 50, 255};
    const Color settingsColor = CheckCollisionPointRec(GetMousePosition(), settingsBtn)
                              ? Color{100, 100, 100, 255}
                              : Color{50, 50, 50, 255};

    DrawRectangleRec(enterGameBtn, enterColor);
    const Vector2 enterTextSize = MeasureTextEx(ctx.font, "Enter Game", 20, 1.0f);
    DrawTextEx(ctx.font, "Enter Game",
               {static_cast<float>(screenWidth) / 2 - enterTextSize.x / 2,
                static_cast<float>(screenHeight) / 2 - 35},
               20, 1.0f, WHITE);

    DrawRectangleRec(settingsBtn, settingsColor);
    const Vector2 settingsTextSize = MeasureTextEx(ctx.font, "Settings", 20, 1.0f);
    DrawTextEx(ctx.font, "Settings",
               {static_cast<float>(screenWidth) / 2 - settingsTextSize.x / 2,
                static_cast<float>(screenHeight) / 2 + 25},
               20, 1.0f, WHITE);

    constexpr const char *title = "The Raven";
    const Vector2 titleSize = MeasureTextEx(ctx.font, title, 40, 1.0f);
    DrawTextEx(ctx.font, title,
               {static_cast<float>(screenWidth) / 2 - titleSize.x / 2, 100.0f}, 40, 1.0f,
               WHITE);

  } else if (ctx.state == GameState::GAME) {
    ClearBackground(Color{15, 15, 20, 255});

    BeginMode3D(ctx.camera);

    DrawSky(ctx, ctx.camera);

    [[maybe_unused]] int culled = 0;
    [[maybe_unused]] int rendered = 0;

    for (auto &[coords, chunk] : ctx.chunks) {
      if (!IsChunkInFrustum(chunk, ctx.camera)) {
        ++culled;
        continue;
      }

      const Vector3 chunkPos = {static_cast<float>(chunk.x * 31), 0.0f,
                                static_cast<float>(chunk.z * 31)};
      DrawModel(chunk.model, chunkPos, 1.0f, WHITE);

      DrawVegetation(ctx, chunk, ctx.camera);

      ++rendered;
    }

    DrawModel(ctx.hutModel, ctx.spawnHut.position, 1.0f, WHITE);
    // DrawGrid(100, 10.0f);
    EndMode3D();

    DrawFPSCounter(ctx);

  } else if (ctx.state == GameState::SETTINGS) {
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();

    constexpr const char *settingsTitle = "Settings";
    const Vector2 titleSize = MeasureTextEx(ctx.font, settingsTitle, 30, 1.0f);
    DrawTextEx(ctx.font, settingsTitle,
               {static_cast<float>(screenWidth) / 2 - titleSize.x / 2, 100.0f}, 30, 1.0f,
               WHITE);

    constexpr const char *placeholder = "Settings coming soon...";
    const Vector2 textSize = MeasureTextEx(ctx.font, placeholder, 20, 1.0f);
    DrawTextEx(ctx.font, placeholder,
               {static_cast<float>(screenWidth) / 2 - textSize.x / 2, 
                static_cast<float>(screenHeight) / 2},
               20, 1.0f, LIGHTGRAY);

    constexpr const char *escText = "Press ESC to return";
    const Vector2 escSize = MeasureTextEx(ctx.font, escText, 16, 1.0f);
    DrawTextEx(ctx.font, escText,
               {static_cast<float>(screenWidth) / 2 - escSize.x / 2, 
                static_cast<float>(screenHeight - 50)},
               16, 1.0f, GRAY);
  }
}

void UnloadGame(GameContext& ctx) {
  std::cout << "Game Unloaded" << std::endl;

  CleanupSky(ctx);
  UnloadVegetationModels(ctx);
  UnloadWater(ctx);
  UnloadModel(ctx.hutModel);

  for (auto &[coords, chunk] : ctx.chunks) {
    UnloadModel(chunk.model);
  }
  ctx.chunks.clear();

  UnloadShader(ctx.lightingShader);
  UnloadFont(ctx.font);
}

// FPS Counter with smoothing to make it more readable
void DrawFPSCounter(const GameContext& ctx) {
  static std::array<float, 120> fpsHistory{};
  static int fpsIndex = 0;
  static bool fpsInitialized = false;

  // Get current FPS from Raylib
  const int currentFps = GetFPS();

  // Initialize the history array if needed
  if (!fpsInitialized) {
    fpsHistory.fill(static_cast<float>(currentFps));
    fpsInitialized = true;
  }

  fpsHistory[fpsIndex] = static_cast<float>(currentFps);
  fpsIndex = (fpsIndex + 1) % fpsHistory.size();

  float sum = 0.0f;
  for (int i = 0; i < 60; ++i) {
    sum += fpsHistory[i];
  }
  const float avgFps = sum / 60.0f;

  const std::string fpsText = std::format("FPS: {:.0f}", avgFps);
  DrawText(fpsText.c_str(), 10, 10, 20, YELLOW);

  // Draw player position (XYZ)
  const std::string posText = std::format("XYZ: ({:.1f}, {:.1f}, {:.1f})", 
                                          ctx.camera.position.x, ctx.camera.position.y, ctx.camera.position.z);
  DrawText(posText.c_str(), 10, 35, 20, YELLOW);

  // Draw distance from spawn
  const float distFromSpawn = Vector3Distance(ctx.camera.position, ctx.spawnHut.position);
  const std::string distText = std::format("Distance from spawn: {:.1f}", distFromSpawn);
  DrawText(distText.c_str(), 10, 60, 20, YELLOW);
}
