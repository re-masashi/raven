#include "db_perlin.hpp"
#include "game.h"
#include "rlgl.h"
#include <cmath>
#include <iostream>
#include <vector>

bool vegetationLoaded = false;
extern Shader lightingShader;

void LoadVegetationModels() {
  vegetationLoaded = true;
  std::cout << "Vegetation ready (procedural)" << std::endl;
}

[[nodiscard]] bool isOnPathVeg(float wx, float wz) noexcept {
  const float pathNoise1 =
      db::perlin(wx * 0.015f + 1000.0f, wz * 0.015f + 1000.0f);
  const float pathNoise2 =
      db::perlin(wx * 0.02f + 2000.0f, wz * 0.02f + 2000.0f);
  const float pathValue = std::abs(pathNoise1) + std::abs(pathNoise2) * 0.5f;

  if (pathValue < 0.15f) {
    return true;
  }

  const float secondaryPath =
      db::perlin(wx * 0.01f + 5000.0f, wz * 0.01f + 5000.0f);
  return std::abs(secondaryPath) < 0.08f;
}

void GenerateVegetationForChunk(Chunk &chunk) {
  return; // no vegetation for now
  if (!vegetationLoaded) {
    return;
  }

  chunk.vegetation.clear();

  constexpr int chunkSize = 32;
  constexpr int stride = 31;

  // Sample every 2-3 vertices
  for (int z = 2; z < chunkSize - 2; z += 3) {
    for (int x = 2; x < chunkSize - 2; x += 3) {
      const int idx = z * chunkSize + x;
      const float height = chunk.heights[idx];

      const float wx = static_cast<float>(chunk.x * stride + x);
      const float wz = static_cast<float>(chunk.z * stride + z);

      // Skip paths
      if (isOnPathVeg(wx, wz)) {
        continue;
      }

      const float placementNoise =
          db::perlin(wx * 0.3f + 3000.0f, wz * 0.3f + 3000.0f);

      // Only place grass sometimes
      if (placementNoise < -2.0f) {
        continue;
      }

      const float jitterX =
          db::perlin(wx * 0.5f + 5000.0f, wz * 0.5f + 5000.0f) * 1.5f;
      const float jitterZ =
          db::perlin(wx * 0.5f + 6000.0f, wz * 0.5f + 6000.0f) * 1.5f;

      VegetationInstance veg;
      veg.position = {static_cast<float>(chunk.x * stride + x) + jitterX,
                      height * 5.0f,
                      static_cast<float>(chunk.z * stride + z) + jitterZ};
      veg.rotation = placementNoise * 360.0f;
      veg.scale = 0.3f + placementNoise * 0.2f;
      veg.modelType = 0;
      chunk.vegetation.emplace_back(veg);
    }
  }
}

void DrawVegetation(const Chunk &chunk, const Camera &camera) {
  return; // no vegetation for now
  if (!vegetationLoaded) {
    return;
  }

  constexpr float maxDrawDistance = 150.0f;

  for (const auto &veg : chunk.vegetation) {
    const float dist = Vector3Distance(camera.position, veg.position);
    if (dist > maxDrawDistance) {
      continue;
    }

    const Vector3 grassPos = {veg.position.x, veg.position.y + 0.05f,
                              veg.position.z};

    // Dead grass color
    constexpr Color grassColor = {75, 70, 40, 200};

    rlPushMatrix();
    rlTranslatef(grassPos.x, grassPos.y, grassPos.z);
    rlRotatef(90, 1, 0, 0);           // Rotate to lay flat
    rlRotatef(veg.rotation, 0, 0, 1); // Random rotation

    DrawCubeV(Vector3{0, 0, 0},
              Vector3{veg.scale * 0.8f, veg.scale * 0.6f, 0.02f}, grassColor);

    rlPopMatrix();
  }
}

void UnloadVegetationModels() {
  vegetationLoaded = false;
  std::cout << "Vegetation unloaded" << std::endl;
}
