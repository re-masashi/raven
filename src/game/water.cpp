// src/game/water.cpp
#include "game.h"
#include "rlgl.h"
#include <iostream>

Model waterPlane;
float waterLevel = 10.0f;

extern Shader lightingShader;

void InitWater() {
  // Create large water plane
  Mesh waterMesh = GenMeshPlane(2000.0f, 2000.0f, 1, 1);
  waterPlane = LoadModelFromMesh(waterMesh);

  // Apply fog shader
  waterPlane.materials[0].shader = lightingShader;

  // Water color
  waterPlane.materials[0].maps[MATERIAL_MAP_DIFFUSE].color =
      (Color){20, 30, 100, 180};

  std::cout << "Water initialized at level: " << waterLevel << std::endl;
}

void DrawWater(const Camera &camera) {
  // Draw water plane at fixed height, centered on camera
  Vector3 waterPos = {camera.position.x, waterLevel, camera.position.z};

  // Enable transparency
  rlSetBlendMode(BLEND_ALPHA);
  DrawModel(waterPlane, waterPos, 1.0f, WHITE);
  rlSetBlendMode(BLEND_ALPHA);
}

void UnloadWater() {
  UnloadModel(waterPlane);
  std::cout << "Water unloaded" << std::endl;
}
