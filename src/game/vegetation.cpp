#include "db_perlin.hpp"
#include "game.h"
#include "rlgl.h"
#include <iostream>
#include <vector>

bool vegetationLoaded = false;
extern Shader lightingShader;

void LoadVegetationModels() {
  vegetationLoaded = true;
  std::cout << "Vegetation ready (procedural)" << std::endl;
}

bool isOnPathVeg(float wx, float wz) {
  float pathNoise1 = db::perlin(wx * 0.015f + 1000.0f, wz * 0.015f + 1000.0f);
  float pathNoise2 = db::perlin(wx * 0.02f + 2000.0f, wz * 0.02f + 2000.0f);
  float pathValue = fabs(pathNoise1) + fabs(pathNoise2) * 0.5f;

  if (pathValue < 0.15f)
    return true;

  float secondaryPath = db::perlin(wx * 0.01f + 5000.0f, wz * 0.01f + 5000.0f);
  return fabs(secondaryPath) < 0.08f;
}

void GenerateVegetationForChunk(Chunk &chunk) {
  if (!vegetationLoaded)
    return;

  chunk.vegetation.clear();

  const int chunkSize = 32;
  const int stride = 31;

  // Sample every 2-3 vertices
  for (int z = 2; z < chunkSize - 2; z += 3) {
    for (int x = 2; x < chunkSize - 2; x += 3) {
      int idx = z * chunkSize + x;
      float height = chunk.heights[idx];

      float wx = chunk.x * stride + x;
      float wz = chunk.z * stride + z;

      // Skip paths
      if (isOnPathVeg(wx, wz))
        continue;

      float placementNoise =
          db::perlin(wx * 0.3f + 3000.0f, wz * 0.3f + 3000.0f);

      // Only place grass sometimes
      if (placementNoise < -2.0f)
        continue;

      float jitterX =
          db::perlin(wx * 0.5f + 5000.0f, wz * 0.5f + 5000.0f) * 1.5f;
      float jitterZ =
          db::perlin(wx * 0.5f + 6000.0f, wz * 0.5f + 6000.0f) * 1.5f;

      VegetationInstance veg;
      veg.position = {chunk.x * (float)stride + x + jitterX, height * 5.0f,
                      chunk.z * (float)stride + z + jitterZ};
      veg.rotation = placementNoise * 360.0f;
      veg.scale = 0.3f + placementNoise * 0.2f;
      veg.modelType = 0;
      chunk.vegetation.push_back(veg);
    }
  }
}

// src/game/vegetation.cpp

void DrawVegetation(const Chunk &chunk, const Camera &camera) {
  if (!vegetationLoaded)
    return;

  const float maxDrawDistance = 150.0f;

  for (const auto &veg : chunk.vegetation) {
    float dist = Vector3Distance(camera.position, veg.position);
    if (dist > maxDrawDistance)
      continue;

    Vector3 grassPos = {veg.position.x, veg.position.y + 0.05f, veg.position.z};

    // Dead grass color
    Color grassColor = (Color){75, 70, 40, 200};

    rlPushMatrix();
    rlTranslatef(grassPos.x, grassPos.y, grassPos.z);
    rlRotatef(90, 1, 0, 0);           // Rotate to lay flat
    rlRotatef(veg.rotation, 0, 0, 1); // Random rotation

    DrawCubeV((Vector3){0, 0, 0},
              (Vector3){veg.scale * 0.8f, veg.scale * 0.6f, 0.02f}, grassColor);

    rlPopMatrix();
  }
}

void UnloadVegetationModels() {
  vegetationLoaded = false;
  std::cout << "Vegetation unloaded" << std::endl;
}
