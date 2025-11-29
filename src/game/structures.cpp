// src/game/structures.cpp
#include "game.h"
#include <iostream>

Structure spawnHut;
Model hutModel;

extern Shader lightingShader;

void initializeSpawnHut() {
  spawnHut.position = (Vector3){12, 0, -15};
  spawnHut.size = (Vector3){8.0f, 6.0f, 8.0f};
  spawnHut.color = (Color){80, 45, 40, 255};

  // Create mesh for hut
  Mesh hutMesh = GenMeshCube(spawnHut.size.x, spawnHut.size.y, spawnHut.size.z);

  // Add vertex colors to mesh
  hutMesh.colors =
      (unsigned char *)malloc(hutMesh.vertexCount * 4 * sizeof(unsigned char));
  for (int i = 0; i < hutMesh.vertexCount; i++) {
    hutMesh.colors[i * 4] = spawnHut.color.r;
    hutMesh.colors[i * 4 + 1] = spawnHut.color.g;
    hutMesh.colors[i * 4 + 2] = spawnHut.color.b;
    hutMesh.colors[i * 4 + 3] = 255;
  }

  UploadMesh(&hutMesh, false);
  hutModel = LoadModelFromMesh(hutMesh);
  hutModel.materials[0].shader = lightingShader; // Apply fog shader

  // Adjust Y position
  float terrainHeight = 15.0f; // Approximate
  spawnHut.position.y = terrainHeight + spawnHut.size.y / 2.0f;

  std::cout << "Spawn hut placed at: " << spawnHut.position.x << ", "
            << spawnHut.position.y << ", " << spawnHut.position.z << std::endl;
}

void UnloadHut() { UnloadModel(hutModel); }
