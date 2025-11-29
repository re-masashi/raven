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

struct Star {
  Vector3 position;
  float brightness;
  float size;
};

// Player animation state
struct PlayerAnimationState {
  float walkCycleTimer = 0.0f;
  float landingTimer = 0.0f;
  float velocitySmooth = 0.0f;
  float breathTimer = 0.0f;
  Vector3 headOffset{0, 0, 0};
  Vector3 headVelocity{0, 0, 0};
};

// Encapsulated game state - no more globals!
struct GameContext {
  // Rendering resources
  Font font{};
  Camera camera{};
  Shader lightingShader{};
  
  // Game state
  GameState state = GameState::MENU;
  float cameraYaw = 0.0f;
  float cameraPitch = 0.0f;
  int renderDistance = 3;
  
  // World data
  std::unordered_map<std::pair<int, int>, Chunk, pair_hash> chunks;
  
  // Structures
  Structure spawnHut;
  Model hutModel;
  
  // Player state
  Vector3 playerVelocity{0, 0, 0};
  float playerHeight = 1.7f;
  bool isGrounded = false;
  PlayerAnimationState playerAnim;
  
  // Sky
  std::vector<Star> stars;
  
  // Water
  Model waterPlane;
  
  // Vegetation
  bool vegetationLoaded = false;
};

void InitGame(GameContext& ctx);
void UpdateGame(GameContext& ctx);
void DrawGame(GameContext& ctx);
void UnloadGame(GameContext& ctx);
void generateChunk(GameContext& ctx, int cx, int cz);
float getTerrainHeight(const GameContext& ctx, float wx, float wz);
[[nodiscard]] bool IsChunkInFrustum(const Chunk &chunk, const Camera &camera) noexcept;

void UpdatePlayer(GameContext& ctx, float deltaTime);
void initializeSpawnHut(GameContext& ctx);

void GenerateStars(GameContext& ctx);
void DrawSky(const GameContext& ctx, const Camera& camera);
void CleanupSky(GameContext& ctx);

void LoadVegetationModels(GameContext& ctx);
void GenerateVegetationForChunk(const GameContext& ctx, Chunk &chunk);
void DrawVegetation(const GameContext& ctx, const Chunk &chunk, const Camera &camera);
void UnloadVegetationModels(GameContext& ctx);

void InitWater(GameContext& ctx);
void DrawWater(const GameContext& ctx, const Camera &camera);
void UnloadWater(GameContext& ctx);

void DrawFPSCounter(const GameContext& ctx);
