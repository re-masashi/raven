#include "game.h"
#include <iostream>

void initializeSpawnHut(GameContext& ctx) {
  ctx.spawnHut.position = {12, 0, -15};
  ctx.spawnHut.size = {8.0f, 6.0f, 8.0f};
  ctx.spawnHut.color = {80, 45, 40, 255};

  // Create mesh for hut
  Mesh hutMesh = GenMeshCube(ctx.spawnHut.size.x, ctx.spawnHut.size.y, ctx.spawnHut.size.z);

  // Add vertex colors to mesh
  hutMesh.colors = static_cast<unsigned char*>(MemAlloc(static_cast<unsigned int>(hutMesh.vertexCount * 4 * sizeof(unsigned char))));
  for (int i = 0; i < hutMesh.vertexCount; ++i) {
    hutMesh.colors[i * 4] = ctx.spawnHut.color.r;
    hutMesh.colors[i * 4 + 1] = ctx.spawnHut.color.g;
    hutMesh.colors[i * 4 + 2] = ctx.spawnHut.color.b;
    hutMesh.colors[i * 4 + 3] = 255;
  }

  UploadMesh(&hutMesh, false);
  ctx.hutModel = LoadModelFromMesh(hutMesh);
  ctx.hutModel.materials[0].shader = ctx.lightingShader; // Apply fog shader

  constexpr float terrainHeight = 15.0f; // Approximate
  ctx.spawnHut.position.y = terrainHeight + ctx.spawnHut.size.y / 2.0f;

  std::cout << "Spawn hut placed at: " << ctx.spawnHut.position.x << ", "
            << ctx.spawnHut.position.y << ", " << ctx.spawnHut.position.z << std::endl;
}
