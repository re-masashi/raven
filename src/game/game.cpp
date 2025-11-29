#ifndef DB_PERLIN_IMPL
#define DB_PERLIN_IMPL
#include "db_perlin.hpp"
#endif // !DEBUG

#include "game.h"
#include "raymath.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <vector>

Font font;
Camera camera = {0};
Shader lightingShader = {0};
GameState state = MENU;
float cameraYaw = 0.0f;
float cameraPitch = 0.0f;
int renderDistance = 3;

std::unordered_map<std::pair<int, int>, Chunk, pair_hash> chunks;

void InitGame() {
  std::cout << "Game Initialized" << std::endl;

  font = LoadFontEx("/usr/share/fonts/TTF/Roboto-Medium.ttf", 20, NULL, 0);

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

  Vector3 lightDir = {-0.4f, -0.8f, 0.4f};
  float len = sqrt(lightDir.x * lightDir.x + lightDir.y * lightDir.y +
                   lightDir.z * lightDir.z);
  lightDir.x /= len;
  lightDir.y /= len;
  lightDir.z /= len;

  Vector3 lightColor = (Vector3){1.0f, 0.85f, 1.0f};

  SetShaderValue(lightingShader, GetShaderLocation(lightingShader, "lightDir"),
                 &lightDir, SHADER_UNIFORM_VEC3);
  SetShaderValue(lightingShader,
                 GetShaderLocation(lightingShader, "lightColor"), &lightColor,
                 SHADER_UNIFORM_VEC3);
  initializeSpawnHut();

  camera.position =
      (Vector3){spawnHut.position.x - 15.0f, spawnHut.position.y + 10.0f,
                spawnHut.position.z - 15.0f};
  camera.target = spawnHut.position;
  camera.up = (Vector3){0.0f, 1.0f, 0.0f};
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
  if (state == MENU) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    Rectangle enterGameBtn = {((float)screenWidth / 2 - 150),
                              ((float)screenHeight / 2 - 50), 300, 50};
    Rectangle settingsBtn = {((float)screenWidth / 2 - 150),
                             ((float)screenHeight / 2 + 10), 300, 50};

    if (CheckCollisionPointRec(GetMousePosition(), enterGameBtn) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      state = GAME;
      DisableCursor();
    }
    if (CheckCollisionPointRec(GetMousePosition(), settingsBtn) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      state = SETTINGS;
    }
    if (IsKeyPressed(KEY_G)) {
      state = GAME;
      DisableCursor();
    }
  } else if (state == GAME) {
    if (IsKeyPressed(KEY_ESCAPE)) {
      state = MENU;
      EnableCursor();
      return;
    }

    // Mouse look
    Vector2 mouseDelta = GetMouseDelta();
    float sensitivity = 0.003f;
    cameraYaw -= mouseDelta.x * sensitivity;
    cameraPitch -= mouseDelta.y * sensitivity;

    if (cameraPitch < -PI / 2 + 0.1f)
      cameraPitch = -PI / 2 + 0.1f;
    if (cameraPitch > PI / 2 - 0.1f)
      cameraPitch = PI / 2 - 0.1f;

    float deltaTime = GetFrameTime();
    UpdatePlayer(deltaTime);

    // Teleport to spawn with H key
    if (IsKeyPressed(KEY_H)) {
      camera.position =
          (Vector3){spawnHut.position.x - 15.0f, spawnHut.position.y + 10.0f,
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
    const int stride = 31;
    int cx = (int)floor(camera.position.x / stride);
    int cz = (int)floor(camera.position.z / stride);

    for (int dx = -renderDistance; dx <= renderDistance; dx++) {
      for (int dz = -renderDistance; dz <= renderDistance; dz++) {
        int ncx = cx + dx;
        int ncz = cz + dz;

        if (chunks.find({ncx, ncz}) == chunks.end()) {
          generateChunk(ncx, ncz);
          GenerateVegetationForChunk(chunks[{ncx, ncz}]);
        }
      }
    }

    // Unload distant chunks
    const float unloadDistance = (renderDistance + 2) * stride;
    std::vector<std::pair<int, int>> toUnload;

    for (auto &p : chunks) {
      Vector3 chunkCenter = {(float)(p.second.x * stride + stride / 2), 15.0f,
                             (float)(p.second.z * stride + stride / 2)};
      float dist = Vector3Distance(camera.position, chunkCenter);

      if (dist > unloadDistance) {
        toUnload.push_back({p.first.first, p.first.second});
      }
    }

    for (auto &key : toUnload) {
      UnloadModel(chunks[key].model);
      chunks.erase(key);
    }
  } else if (state == SETTINGS) {
    if (IsKeyPressed(KEY_ESCAPE)) {
      state = MENU;
    }
  }
}

void DrawGame() {
  if (state == MENU) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    Rectangle enterGameBtn = {((float)screenWidth / 2 - 150),
                              ((float)screenHeight / 2 - 50), 300, 50};
    Rectangle settingsBtn = {((float)screenWidth / 2 - 150),
                             ((float)screenHeight / 2 + 10), 300, 50};

    Color enterColor = CheckCollisionPointRec(GetMousePosition(), enterGameBtn)
                           ? (Color){100, 100, 100, 255}
                           : (Color){50, 50, 50, 255};
    Color settingsColor =
        CheckCollisionPointRec(GetMousePosition(), settingsBtn)
            ? (Color){100, 100, 100, 255}
            : (Color){50, 50, 50, 255};

    DrawRectangleRec(enterGameBtn, enterColor);
    Vector2 enterTextSize = MeasureTextEx(font, "Enter Game", 20, 1.0f);
    DrawTextEx(font, "Enter Game",
               {((float)screenWidth / 2 - enterTextSize.x / 2),
                ((float)screenHeight / 2 - 35)},
               20, 1.0f, WHITE);

    DrawRectangleRec(settingsBtn, settingsColor);
    Vector2 settingsTextSize = MeasureTextEx(font, "Settings", 20, 1.0f);
    DrawTextEx(font, "Settings",
               {((float)screenWidth / 2 - settingsTextSize.x / 2),
                ((float)screenHeight / 2 + 25)},
               20, 1.0f, WHITE);

    const char *title = "The Raven";
    Vector2 titleSize = MeasureTextEx(font, title, 40, 1.0f);
    DrawTextEx(font, title,
               {((float)screenWidth / 2 - titleSize.x / 2), 100.0f}, 40, 1.0f,
               WHITE);

  } else if (state == GAME) {
    ClearBackground((Color){15, 15, 20, 255});

    BeginMode3D(camera);

    DrawSky(camera);

    int culled = 0;
    int rendered = 0;

    for (auto &p : chunks) {
      if (!IsChunkInFrustum(p.second, camera)) {
        culled++;
        continue;
      }

      Vector3 chunkPos = {(float)(p.second.x * 31), 0.0f,
                          (float)(p.second.z * 31)};
      DrawModel(p.second.model, chunkPos, 1.0f, WHITE);

      DrawVegetation(p.second, camera);

      rendered++;
    }

    DrawModel(hutModel, spawnHut.position, 1.0f, WHITE);
    // DrawGrid(100, 10.0f);
    EndMode3D();

    DrawFPSCounter();

  } else if (state == SETTINGS) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    const char *settingsTitle = "Settings";
    Vector2 titleSize = MeasureTextEx(font, settingsTitle, 30, 1.0f);
    DrawTextEx(font, settingsTitle,
               {((float)screenWidth / 2 - titleSize.x / 2), 100.0f}, 30, 1.0f,
               WHITE);

    const char *placeholder = "Settings coming soon...";
    Vector2 textSize = MeasureTextEx(font, placeholder, 20, 1.0f);
    DrawTextEx(
        font, placeholder,
        {((float)screenWidth / 2 - textSize.x / 2), ((float)screenHeight / 2)},
        20, 1.0f, LIGHTGRAY);

    const char *escText = "Press ESC to return";
    Vector2 escSize = MeasureTextEx(font, escText, 16, 1.0f);
    DrawTextEx(
        font, escText,
        {((float)screenWidth / 2 - escSize.x / 2), ((float)screenHeight - 50)},
        16, 1.0f, GRAY);
  }
}

void UnloadGame() {
  std::cout << "Game Unloaded" << std::endl;

  CleanupSky();
  UnloadVegetationModels();
  UnloadWater();
  UnloadHut();

  for (auto &p : chunks) {
    UnloadModel(p.second.model);
  }
  chunks.clear();

  UnloadShader(lightingShader);
  UnloadFont(font);
}

// FPS Counter with smoothing to make it more readable
static float fpsHistory[120] = {0};
static int fpsIndex = 0;
static bool fpsInitialized = false;

void DrawFPSCounter() {
  // Get current FPS from Raylib
  int currentFps = GetFPS();

  // Initialize the history array if needed
  if (!fpsInitialized) {
    for (int i = 0; i < 60; i++) {
      fpsHistory[i] = (float)currentFps;
    }
    fpsInitialized = true;
  }

  fpsHistory[fpsIndex] = (float)currentFps;
  fpsIndex =
      (fpsIndex + 1) %
      (sizeof(fpsHistory) / sizeof(float)); // Move to next index, wrap around

  float sum = 0.0f;
  for (int i = 0; i < 60; i++) {
    sum += fpsHistory[i];
  }
  float avgFps = sum / 60.0f;

  char fpsText[32];
  sprintf(fpsText, "FPS: %.0f", avgFps);

  DrawText(fpsText, 10, 10, 20, YELLOW);

  // Draw player position (XYZ)
  char posText[64];
  sprintf(posText, "XYZ: (%.1f, %.1f, %.1f)", camera.position.x, camera.position.y, camera.position.z);
  DrawText(posText, 10, 35, 20, YELLOW);

  // Draw distance from spawn
  float distFromSpawn = Vector3Distance(camera.position, spawnHut.position);
  char distText[64];
  sprintf(distText, "Distance from spawn: %.1f", distFromSpawn);
  DrawText(distText, 10, 60, 20, YELLOW);
}
