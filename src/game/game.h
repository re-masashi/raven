#pragma once

#include "raylib.h"
#include "raymath.h"
#include <unordered_map>
#include <vector>
#include <array>
#include <span>
#include <memory>

enum class GameState { MENU, GAME, SETTINGS };

struct Path {
  std::vector<Vector3> points;
  float width;
};

struct Structure {
  Vector3 position;
  Vector3 size;
  Color color;
  int type;
};

struct VegetationInstance {
  Vector3 position;
  float rotation;
  float scale;
  int modelType; // 0=stump, 1=oak, 2=lowpoly, 3=grass
};

struct Chunk {
  int x, z;
  Model model;
  std::vector<float> heights;
  std::vector<float> moisture;
  std::vector<VegetationInstance> vegetation;
};

struct pair_hash {
  template <class T1, class T2>
  [[nodiscard]] constexpr std::size_t operator()(const std::pair<T1, T2> &pair) const noexcept {
    return std::hash<T1>()(pair.first) ^ (std::hash<T2>()(pair.second) << 1);
  }
};

void InitGame();
void UpdateGame();
void DrawGame();
void UnloadGame();
void generateChunk(int cx, int cz);
float getTerrainHeight(float wx, float wz);
[[nodiscard]] bool IsChunkInFrustum(const Chunk &chunk, const Camera &camera) noexcept;

void UpdatePlayer(float deltaTime);
void initializeSpawnHut();
float getTerrainHeight(float worldX, float worldZ);
void UnloadHut();

void GenerateStars();
void DrawSky(Camera camera);
void CleanupSky();

void LoadVegetationModels();
void GenerateVegetationForChunk(Chunk &chunk);
void DrawVegetation(const Chunk &chunk, const Camera &camera);
void UnloadVegetationModels();

void InitWater();
void DrawWater(const Camera &camera);
void UnloadWater();

void DrawFPSCounter();

// World boundaries
constexpr Vector3 WORLD_CENTER = {0.0f, 0.0f, 0.0f};
constexpr float WORLD_RADIUS = 750.0f;
constexpr float SOFT_BOUNDARY_START = 650.0f; // Start gentle push
constexpr float HARD_BOUNDARY_START = 850.0f; // Strong push back (100 units past radius)

void ApplyWorldBoundaries(float deltaTime);
void DrawBoundaryWarning();

// Global structures
inline Structure spawnHut;
inline Model hutModel;
inline Vector3 playerVelocity{0, 0, 0};
inline float playerHeight = 1.7f;
inline bool isGrounded = false;
