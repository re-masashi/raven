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

Font font{};
Camera camera{};
Shader lightingShader{};
GameState state = GameState::MENU;
float mouseSensitivity = 0.003f;
float cameraYaw = 0.0f;
float cameraPitch = 0.0f;
int renderDistance = 3;

std::unordered_map<std::pair<int, int>, Chunk, pair_hash> chunks;

void InitGame() {
  std::cout << "Game Initialized" << std::endl;

  font = LoadFontEx("/usr/share/fonts/TTF/Roboto-Medium.ttf", 20, nullptr, 0);

  lightingShader =
      LoadShader("src/shaders/vertex.glsl", "src/shaders/fragment.glsl");

  if (lightingShader.id == 0) {
    std::cout << "ERROR: Shader failed to load!" << std::endl;
  } else {
    std::cout << "Shader loaded successfully!" << std::endl;
  }

  GenerateStars();
  LoadVegetationModels();
  InitWater();

  Vector3 lightDir = {-0.9659f, -0.2588f, 0.0f}; // ~15 degrees from horizontal
  const float len =
      std::sqrt(lightDir.x * lightDir.x + lightDir.y * lightDir.y +
                lightDir.z * lightDir.z);
  lightDir.x /= len;
  lightDir.y /= len;
  lightDir.z /= len;

  Vector3 lightColor = {1.1f, 0.9f, 1.1f};

  SetShaderValue(lightingShader, GetShaderLocation(lightingShader, "lightDir"),
                 &lightDir, SHADER_UNIFORM_VEC3);
  SetShaderValue(lightingShader,
                 GetShaderLocation(lightingShader, "lightColor"), &lightColor,
                 SHADER_UNIFORM_VEC3);
  
  // Set world boundary uniforms
  SetShaderValue(lightingShader, GetShaderLocation(lightingShader, "worldCenter"),
                 &WORLD_CENTER, SHADER_UNIFORM_VEC3);
  SetShaderValue(lightingShader, GetShaderLocation(lightingShader, "worldRadius"),
                 &WORLD_RADIUS, SHADER_UNIFORM_FLOAT);
  
  initializeSpawnHut();

  camera.position = {spawnHut.position.x - 15.0f, spawnHut.position.y + 10.0f,
                     spawnHut.position.z - 15.0f};
  camera.target = spawnHut.position;
  camera.up = {0.0f, 1.0f, 0.0f};
  camera.fovy = 45.0f;
  camera.projection = CAMERA_PERSPECTIVE;

  for (int dx = -renderDistance; dx <= renderDistance; dx++) {
    for (int dz = -renderDistance; dz <= renderDistance; dz++) {
      generateChunk(dx, dz);
      GenerateVegetationForChunk(chunks[{dx, dz}]);
    }
  }
}

void UpdateGame() {
  if (state == GameState::MENU) {
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();

    const Rectangle enterGameBtn = {static_cast<float>(screenWidth) / 2 - 150,
                                    static_cast<float>(screenHeight) / 2 - 50,
                                    300, 50};
    const Rectangle settingsBtn = {static_cast<float>(screenWidth) / 2 - 150,
                                   static_cast<float>(screenHeight) / 2 + 10,
                                   300, 50};

    if (CheckCollisionPointRec(GetMousePosition(), enterGameBtn) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      state = GameState::GAME;
      DisableCursor();
    }
    if (CheckCollisionPointRec(GetMousePosition(), settingsBtn) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      state = GameState::SETTINGS;
    }
    if (IsKeyPressed(KEY_G)) {
      state = GameState::GAME;
      DisableCursor();
    }
  } else if (state == GameState::GAME) {
    if (IsKeyPressed(KEY_ESCAPE)) {
      state = GameState::MENU;
      EnableCursor();
      return;
    }

    // Mouse look
    const Vector2 mouseDelta = GetMouseDelta();
    cameraYaw -= mouseDelta.x * mouseSensitivity;
    cameraPitch -= mouseDelta.y * mouseSensitivity;

    constexpr float maxPitch = PI / 2.0f - 0.1f;
    cameraPitch = std::clamp(cameraPitch, -maxPitch, maxPitch);

    const float deltaTime = GetFrameTime();
    UpdatePlayer(deltaTime);

    // Apply world boundaries (soft/hard push)
    ApplyWorldBoundaries(deltaTime);

    // Teleport to spawn with H key
    if (IsKeyPressed(KEY_H)) {
      camera.position = {spawnHut.position.x - 15.0f,
                         spawnHut.position.y + 10.0f,
                         spawnHut.position.z - 15.0f};
    }

    // Render distance
    if (IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_EQUAL)) {
      renderDistance = std::min(renderDistance + 1, 10);
    }
    if (IsKeyPressed(KEY_KP_SUBTRACT) || IsKeyPressed(KEY_MINUS)) {
      renderDistance = std::max(renderDistance - 1, 2);
    }

    SetShaderValue(lightingShader, GetShaderLocation(lightingShader, "viewPos"),
                   &camera.position, SHADER_UNIFORM_VEC3);

    // Chunk loading
    constexpr int stride = 31;
    const int cx = static_cast<int>(std::floor(camera.position.x / stride));
    const int cz = static_cast<int>(std::floor(camera.position.z / stride));

    for (int dx = -renderDistance; dx <= renderDistance; ++dx) {
      for (int dz = -renderDistance; dz <= renderDistance; ++dz) {
        const int ncx = cx + dx;
        const int ncz = cz + dz;

        if (!chunks.contains({ncx, ncz})) {
          generateChunk(ncx, ncz);
          GenerateVegetationForChunk(chunks[{ncx, ncz}]);
        }
      }
    }

    // Unload distant chunks
    const float unloadDistance =
        static_cast<float>((renderDistance + 2) * stride);
    std::vector<std::pair<int, int>> toUnload;

    for (const auto &[coords, chunk] : chunks) {
      const Vector3 chunkCenter = {
          static_cast<float>(chunk.x * stride + stride / 2), 15.0f,
          static_cast<float>(chunk.z * stride + stride / 2)};
      const float dist = Vector3Distance(camera.position, chunkCenter);

      if (dist > unloadDistance) {
        toUnload.emplace_back(coords.first, coords.second);
      }
    }

    for (const auto &key : toUnload) {
      UnloadModel(chunks[key].model);
      chunks.erase(key);
    }
  } else if (state == GameState::SETTINGS) {
    if (IsKeyPressed(KEY_ESCAPE)) {
      state = GameState::MENU;
    }

    if (IsKeyPressed(KEY_LEFT)) {
      mouseSensitivity = std::max(0.001f, mouseSensitivity - 0.001f);
    }
    if (IsKeyPressed(KEY_RIGHT)) {
      mouseSensitivity = std::min(0.01f, mouseSensitivity + 0.001f);
    }
  }
}

void DrawGame() {
  if (state == GameState::MENU) {
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();

    const Rectangle enterGameBtn = {static_cast<float>(screenWidth) / 2 - 150,
                                    static_cast<float>(screenHeight) / 2 - 50,
                                    300, 50};
    const Rectangle settingsBtn = {static_cast<float>(screenWidth) / 2 - 150,
                                   static_cast<float>(screenHeight) / 2 + 10,
                                   300, 50};

    const Color enterColor =
        CheckCollisionPointRec(GetMousePosition(), enterGameBtn)
            ? Color{100, 100, 100, 255}
            : Color{50, 50, 50, 255};
    const Color settingsColor =
        CheckCollisionPointRec(GetMousePosition(), settingsBtn)
            ? Color{100, 100, 100, 255}
            : Color{50, 50, 50, 255};

    DrawRectangleRec(enterGameBtn, enterColor);
    const Vector2 enterTextSize = MeasureTextEx(font, "Enter Game", 20, 1.0f);
    DrawTextEx(font, "Enter Game",
               {static_cast<float>(screenWidth) / 2 - enterTextSize.x / 2,
                static_cast<float>(screenHeight) / 2 - 35},
               20, 1.0f, WHITE);

    DrawRectangleRec(settingsBtn, settingsColor);
    const Vector2 settingsTextSize = MeasureTextEx(font, "Settings", 20, 1.0f);
    DrawTextEx(font, "Settings",
               {static_cast<float>(screenWidth) / 2 - settingsTextSize.x / 2,
                static_cast<float>(screenHeight) / 2 + 25},
               20, 1.0f, WHITE);

    constexpr const char *title = "The Raven";
    const Vector2 titleSize = MeasureTextEx(font, title, 40, 1.0f);
    DrawTextEx(font, title,
               {static_cast<float>(screenWidth) / 2 - titleSize.x / 2, 100.0f},
               40, 1.0f, WHITE);

  } else if (state == GameState::GAME) {
    ClearBackground(Color{15, 15, 20, 255});

    BeginMode3D(camera);

    DrawSky(camera);

    [[maybe_unused]] int culled = 0;
    [[maybe_unused]] int rendered = 0;

    for (auto &[coords, chunk] : chunks) {
      if (!IsChunkInFrustum(chunk, camera)) {
        ++culled;
        continue;
      }

      const Vector3 chunkPos = {static_cast<float>(chunk.x * 31), 0.0f,
                                static_cast<float>(chunk.z * 31)};
      DrawModel(chunk.model, chunkPos, 1.0f, WHITE);

      DrawVegetation(chunk, camera);

      ++rendered;
    }

    DrawModel(hutModel, spawnHut.position, 1.0f, WHITE);
    // DrawGrid(100, 10.0f);
    EndMode3D();

    DrawFPSCounter();
    DrawBoundaryWarning();

  } else if (state == GameState::SETTINGS) {
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();

    constexpr const char *settingsTitle = "Settings";
    const Vector2 titleSize = MeasureTextEx(font, settingsTitle, 30, 1.0f);
    DrawTextEx(font, settingsTitle,
               {static_cast<float>(screenWidth) / 2 - titleSize.x / 2, 100.0f},
               30, 1.0f, WHITE);

    const std::string sensitivityText =
        std::format("Mouse Sensitivity: {:.3f}", mouseSensitivity);
    const Vector2 sensitivityTextSize =
        MeasureTextEx(font, sensitivityText.c_str(), 20, 1.0f);
    DrawTextEx(font, sensitivityText.c_str(),
               {static_cast<float>(screenWidth) / 2 - sensitivityTextSize.x / 2,
                static_cast<float>(screenHeight) / 2},
               20, 1.0f, LIGHTGRAY);

    const std::string instructionsText = "Use LEFT/RIGHT arrows to change";
    const Vector2 instructionsTextSize =
        MeasureTextEx(font, instructionsText.c_str(), 16, 1.0f);
    DrawTextEx(
        font, instructionsText.c_str(),
        {static_cast<float>(screenWidth) / 2 - instructionsTextSize.x / 2,
         static_cast<float>(screenHeight) / 2 + 30},
        16, 1.0f, GRAY);

    constexpr const char *escText = "Press ESC to return";
    const Vector2 escSize = MeasureTextEx(font, escText, 16, 1.0f);
    DrawTextEx(font, escText,
               {static_cast<float>(screenWidth) / 2 - escSize.x / 2,
                static_cast<float>(screenHeight - 50)},
               16, 1.0f, GRAY);
  }
}

void UnloadGame() {
  std::cout << "Game Unloaded" << std::endl;

  CleanupSky();
  UnloadVegetationModels();
  UnloadWater();
  UnloadHut();

  for (auto &[coords, chunk] : chunks) {
    UnloadModel(chunk.model);
  }
  chunks.clear();

  UnloadShader(lightingShader);
  UnloadFont(font);
}

// FPS Counter with smoothing to make it more readable
void DrawFPSCounter() {
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
  const std::string posText =
      std::format("XYZ: ({:.1f}, {:.1f}, {:.1f})", camera.position.x,
                  camera.position.y, camera.position.z);
  DrawText(posText.c_str(), 10, 35, 20, YELLOW);

  // Draw distance from spawn
  const float distFromSpawn =
      Vector3Distance(camera.position, spawnHut.position);
  const std::string distText =
      std::format("Distance from spawn: {:.1f}", distFromSpawn);
  DrawText(distText.c_str(), 10, 60, 20, YELLOW);
}
