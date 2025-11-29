// src/game/water.cpp
#include "game.h"
#include "rlgl.h"
#include <iostream>

constexpr float waterLevel = 10.0f;

void InitWater(GameContext& ctx) {
  // Create large water plane
  Mesh waterMesh = GenMeshPlane(2000.0f, 2000.0f, 1, 1);
  ctx.waterPlane = LoadModelFromMesh(waterMesh);

  // Apply fog shader
  ctx.waterPlane.materials[0].shader = ctx.lightingShader;

  // Water color
  ctx.waterPlane.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = {20, 30, 100, 180};

  std::cout << "Water initialized at level: " << waterLevel << std::endl;
}

void DrawWater(const GameContext& ctx, const Camera &camera) {
  // Draw water plane at fixed height, centered on camera
  const Vector3 waterPos = {camera.position.x, waterLevel, camera.position.z};

  // Enable transparency
  rlSetBlendMode(BLEND_ALPHA);
  DrawModel(ctx.waterPlane, waterPos, 1.0f, WHITE);
  rlSetBlendMode(BLEND_ALPHA);
}

void UnloadWater(GameContext& ctx) {
  UnloadModel(ctx.waterPlane);
  std::cout << "Water unloaded" << std::endl;
}
