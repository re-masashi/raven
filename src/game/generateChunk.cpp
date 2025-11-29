#include "db_perlin.hpp"
#include "game.h"
#include <algorithm>
#include <cmath>
#include <vector>

extern std::unordered_map<std::pair<int, int>, Chunk, pair_hash> chunks;
extern Shader lightingShader;

[[nodiscard]] bool isOnPath(float wx, float wz) noexcept {
  const float pathNoise1 = db::perlin(wx * 0.015f + 1000.0f, wz * 0.015f + 1000.0f);
  const float pathNoise2 = db::perlin(wx * 0.02f + 2000.0f, wz * 0.02f + 2000.0f);

  const float pathValue = std::abs(pathNoise1) + std::abs(pathNoise2) * 0.5f;

  if (pathValue < 0.15f) {
    const float h1 = db::perlin(wx * 0.008f, wz * 0.008f) * 4.0f;
    const float h2 = db::perlin((wx + 2.0f) * 0.008f, wz * 0.008f) * 4.0f;
    const float slope = std::abs(h2 - h1);
    return slope < 0.8f;
  }

  const float secondaryPath = db::perlin(wx * 0.01f + 5000.0f, wz * 0.01f + 5000.0f);
  return std::abs(secondaryPath) < 0.08f;
}

[[nodiscard]] float getPathInfluence(float wx, float wz) noexcept {
  if (!isOnPath(wx, wz)) {
    return 0.0f;
  }

  float influence = 0.7f;

  for (float dx = -1.5f; dx <= 1.5f; dx += 1.5f) {
    for (float dz = -1.5f; dz <= 1.5f; dz += 1.5f) {
      if (dx == 0 && dz == 0) {
        continue;
      }
      if (!isOnPath(wx + dx, wz + dz)) {
        influence *= 0.55f;
      }
    }
  }

  return influence;
}

void generateChunk(int cx, int cz) {
  constexpr int chunkSize = 32;
  constexpr int stride = chunkSize - 1;

  Mesh mesh{};
  mesh.vertexCount = chunkSize * chunkSize;
  mesh.triangleCount = stride * stride * 2;

  // Use new[] for raylib compatibility (needs to be freed with RL_FREE/free)
  mesh.vertices = static_cast<float*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount * 3 * sizeof(float))));
  mesh.texcoords = static_cast<float*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount * 2 * sizeof(float))));
  mesh.normals = static_cast<float*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount * 3 * sizeof(float))));
  mesh.colors = static_cast<unsigned char*>(MemAlloc(static_cast<unsigned int>(mesh.vertexCount * 4 * sizeof(unsigned char))));
  mesh.indices = static_cast<unsigned short*>(MemAlloc(static_cast<unsigned int>(mesh.triangleCount * 3 * sizeof(unsigned short))));

  std::vector<float> heights(chunkSize * chunkSize);
  std::vector<float> moisture(chunkSize * chunkSize);
  std::vector<float> pathInfluence(chunkSize * chunkSize);

  for (int z = 0; z < chunkSize; ++z) {
    for (int x = 0; x < chunkSize; ++x) {
      const float wx = static_cast<float>(cx * stride + x);
      const float wz = static_cast<float>(cz * stride + z);
      const int idx = z * chunkSize + x;

      // MUCH LARGER scale terrain features - frequencies reduced dramatically
      // Base terrain - very large rolling hills
      float height = db::perlin(wx * 0.0015f, wz * 0.0015f) * 8.0f;

      // Secondary large features (mountains/plateaus)
      height += db::perlin(wx * 0.003f + 100.0f, wz * 0.003f + 100.0f) * 4.0f;

      // Medium variation (gentle slopes)
      height += db::perlin(wx * 0.006f + 200.0f, wz * 0.006f + 200.0f) * 1.5f;

      // Fine detail (very subtle - barely noticeable)
      height += db::perlin(wx * 0.015f + 300.0f, wz * 0.015f + 300.0f) * 0.4f;

      // Very wide valleys
      const float valleyNoise = db::perlin(wx * 0.001f + 500.0f, wz * 0.001f + 500.0f);
      if (valleyNoise < -0.25f) {
        height += (valleyNoise + 0.25f) * 3.0f; // Gentle broad valleys
      }

      heights[idx] = height + 3.0f;

      // Moisture
      moisture[idx] = 0.5f + db::perlin(wx * 0.008f + 2000.0f, wz * 0.008f + 2000.0f) * 0.3f;
      if (heights[idx] < 2.5f) {
        moisture[idx] += 0.3f;
      }
      moisture[idx] = std::clamp(moisture[idx], 0.0f, 1.0f);

      pathInfluence[idx] = getPathInfluence(wx, wz);
    }
  }

  // Reduced smoothing - the noise is already smooth at this scale
  constexpr int smoothIterations = 4; // Less aggressive smoothing
  constexpr int edgeMargin = 1;

  for (int iter = 0; iter < smoothIterations; ++iter) {
    std::vector<float> newHeights = heights;
    for (int z = edgeMargin; z < chunkSize - edgeMargin; ++z) {
      for (int x = edgeMargin; x < chunkSize - edgeMargin; ++x) {
        const int idx = z * chunkSize + x;

        // Lighter smoothing - preserve more character
        float sum = heights[idx] * 0.5f; // More center weight
        sum += heights[(z - 1) * chunkSize + x] * 0.125f;
        sum += heights[(z + 1) * chunkSize + x] * 0.125f;
        sum += heights[z * chunkSize + (x - 1)] * 0.125f;
        sum += heights[z * chunkSize + (x + 1)] * 0.125f;
        newHeights[idx] = sum;
      }
    }
    heights = newHeights;
  }

  // Generate mesh vertices with path coloring
  for (int z = 0; z < chunkSize; ++z) {
    for (int x = 0; x < chunkSize; ++x) {
      const int idx = z * chunkSize + x;
      const float height = heights[idx];
      const float m = moisture[idx];
      const float pathVal = pathInfluence[idx];

      mesh.vertices[idx * 3] = static_cast<float>(x);
      mesh.vertices[idx * 3 + 1] = height * 5.0f;
      mesh.vertices[idx * 3 + 2] = static_cast<float>(z);

      mesh.texcoords[idx * 2] = static_cast<float>(x) / stride;
      mesh.texcoords[idx * 2 + 1] = static_cast<float>(z) / stride;

      unsigned char r, g, b;

      if (height < 1.5f) {
        r = static_cast<unsigned char>(45 + static_cast<int>(m * 15.0f));
        g = static_cast<unsigned char>(50 + static_cast<int>(m * 20.0f));
        b = static_cast<unsigned char>(35 + static_cast<int>(m * 10.0f));
      } else if (height < 3.5f) {
        r = static_cast<unsigned char>(60 + static_cast<int>(m * 15.0f));
        g = static_cast<unsigned char>(65 + static_cast<int>(m * 20.0f));
        b = 45;
      } else if (height < 5.0f) {
        r = 70;
        g = 68;
        b = 55;
      } else {
        r = 75;
        g = 70;
        b = 65;
      }

      if (pathVal > 0.0f) {
        constexpr unsigned char pathR = 80;
        constexpr unsigned char pathG = 75;
        constexpr unsigned char pathB = 60;

        r = static_cast<unsigned char>(r * (1.0f - pathVal) + pathR * pathVal);
        g = static_cast<unsigned char>(g * (1.0f - pathVal) + pathG * pathVal);
        b = static_cast<unsigned char>(b * (1.0f - pathVal) + pathB * pathVal);
      }

      mesh.colors[idx * 4] = r;
      mesh.colors[idx * 4 + 1] = g;
      mesh.colors[idx * 4 + 2] = b;
      mesh.colors[idx * 4 + 3] = 255;
    }
  }

  // Compute normals
  for (int z = 0; z < chunkSize; ++z) {
    for (int x = 0; x < chunkSize; ++x) {
      const int idx = z * chunkSize + x;
      const float h = mesh.vertices[idx * 3 + 1];

      const float hx1 = (x > 0) ? mesh.vertices[(z * chunkSize + (x - 1)) * 3 + 1] : h;
      const float hx2 = (x < chunkSize - 1) ? mesh.vertices[(z * chunkSize + (x + 1)) * 3 + 1] : h;
      const float hz1 = (z > 0) ? mesh.vertices[((z - 1) * chunkSize + x) * 3 + 1] : h;
      const float hz2 = (z < chunkSize - 1) ? mesh.vertices[((z + 1) * chunkSize + x) * 3 + 1] : h;

      const float dx = (hx2 - hx1) / 2.0f;
      const float dz = (hz2 - hz1) / 2.0f;

      Vector3 normal = {-dx, 2.0f, -dz};
      const float len = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
      if (len > 0.0f) {
        normal.x /= len;
        normal.y /= len;
        normal.z /= len;
      }

      mesh.normals[idx * 3] = normal.x;
      mesh.normals[idx * 3 + 1] = normal.y;
      mesh.normals[idx * 3 + 2] = normal.z;
    }
  }

  // Generate indices
  int indexCount = 0;
  for (int z = 0; z < stride; ++z) {
    for (int x = 0; x < stride; ++x) {
      const int topLeft = z * chunkSize + x;
      const int topRight = topLeft + 1;
      const int bottomLeft = (z + 1) * chunkSize + x;
      const int bottomRight = bottomLeft + 1;

      mesh.indices[indexCount++] = static_cast<unsigned short>(topLeft);
      mesh.indices[indexCount++] = static_cast<unsigned short>(bottomLeft);
      mesh.indices[indexCount++] = static_cast<unsigned short>(topRight);
      mesh.indices[indexCount++] = static_cast<unsigned short>(topRight);
      mesh.indices[indexCount++] = static_cast<unsigned short>(bottomLeft);
      mesh.indices[indexCount++] = static_cast<unsigned short>(bottomRight);
    }
  }

  UploadMesh(&mesh, false);
  Model model = LoadModelFromMesh(mesh);
  model.materials[0].shader = lightingShader;

  Chunk chunk;
  chunk.x = cx;
  chunk.z = cz;
  chunk.model = model;
  chunk.heights = std::move(heights);
  chunk.moisture = std::move(moisture);
  chunk.vegetation = {}; // Will be populated by GenerateVegetationForChunk
  
  chunks[{cx, cz}] = std::move(chunk);
}

float getTerrainHeight(float wx, float wz) {
  // Determine which chunk this position is in
  constexpr int stride = 31;
  const int cx = static_cast<int>(std::floor(wx / stride));
  const int cz = static_cast<int>(std::floor(wz / stride));

  // Check if chunk exists
  const auto it = chunks.find({cx, cz});
  if (it == chunks.end()) {
    // Chunk not loaded, estimate height from noise
    float height = 0.0f;
    height += db::perlin(wx * 0.004f, wz * 0.004f) * 6.0f;
    height += db::perlin(wx * 0.01f + 100.0f, wz * 0.01f + 100.0f) * 2.5f;
    height += db::perlin(wx * 0.02f + 200.0f, wz * 0.02f + 200.0f) * 0.8f;
    height += db::perlin(wx * 0.05f + 300.0f, wz * 0.05f + 300.0f) * 0.3f;

    const float valleyNoise = db::perlin(wx * 0.003f + 500.0f, wz * 0.003f + 500.0f);
    if (valleyNoise < -0.2f) {
      height += (valleyNoise + 0.2f) * 2.5f;
    }

    return (height + 3.0f) * 5.0f;
  }

  // Get local position within chunk
  float localX = wx - static_cast<float>(cx * stride);
  float localZ = wz - static_cast<float>(cz * stride);

  // Clamp to chunk bounds
  localX = std::clamp(localX, 0.0f, 31.0f);
  localZ = std::clamp(localZ, 0.0f, 31.0f);

  // Get the 4 surrounding vertices for bilinear interpolation
  const int x0 = static_cast<int>(std::floor(localX));
  const int z0 = static_cast<int>(std::floor(localZ));
  const int x1 = std::min(x0 + 1, 31);
  const int z1 = std::min(z0 + 1, 31);

  const float fx = localX - static_cast<float>(x0);
  const float fz = localZ - static_cast<float>(z0);

  // Get heights from chunk data
  const Chunk &chunk = it->second;
  const float h00 = chunk.heights[z0 * 32 + x0];
  const float h10 = chunk.heights[z0 * 32 + x1];
  const float h01 = chunk.heights[z1 * 32 + x0];
  const float h11 = chunk.heights[z1 * 32 + x1];

  // Bilinear interpolation
  const float h0 = h00 * (1.0f - fx) + h10 * fx;
  const float h1 = h01 * (1.0f - fx) + h11 * fx;
  const float height = h0 * (1.0f - fz) + h1 * fz;

  return height * 5.0f; // Scale same as mesh
}
