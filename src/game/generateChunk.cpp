#include "db_perlin.hpp"
#include "game.h"
#include <cmath>
#include <cstdlib>
#include <vector>

extern std::unordered_map<std::pair<int, int>, Chunk, pair_hash> chunks;
extern Shader lightingShader;

bool isOnPath(float wx, float wz) {
  float pathNoise1 = db::perlin(wx * 0.015f + 1000.0f, wz * 0.015f + 1000.0f);
  float pathNoise2 = db::perlin(wx * 0.02f + 2000.0f, wz * 0.02f + 2000.0f);

  float pathValue = fabs(pathNoise1) + fabs(pathNoise2) * 0.5f;

  if (pathValue < 0.15f) {
    float h1 = db::perlin(wx * 0.008f, wz * 0.008f) * 4.0f;
    float h2 = db::perlin((wx + 2.0f) * 0.008f, wz * 0.008f) * 4.0f;
    float slope = fabs(h2 - h1);
    return slope < 0.8f;
  }

  float secondaryPath = db::perlin(wx * 0.01f + 5000.0f, wz * 0.01f + 5000.0f);
  return fabs(secondaryPath) < 0.08f;
}

float getPathInfluence(float wx, float wz) {
  if (!isOnPath(wx, wz))
    return 0.0f;

  float influence = 0.7f;

  for (float dx = -1.5f; dx <= 1.5f; dx += 1.5f) {
    for (float dz = -1.5f; dz <= 1.5f; dz += 1.5f) {
      if (dx == 0 && dz == 0)
        continue;
      if (!isOnPath(wx + dx, wz + dz)) {
        influence *= 0.55f;
      }
    }
  }

  return influence;
}

void generateChunk(int cx, int cz) {
  const int chunkSize = 32;
  const int stride = chunkSize - 1;

  Mesh mesh = {0};
  mesh.vertexCount = chunkSize * chunkSize;
  mesh.triangleCount = stride * stride * 2;

  mesh.vertices = (float *)malloc(mesh.vertexCount * 3 * sizeof(float));
  mesh.texcoords = (float *)malloc(mesh.vertexCount * 2 * sizeof(float));
  mesh.normals = (float *)malloc(mesh.vertexCount * 3 * sizeof(float));
  mesh.colors =
      (unsigned char *)malloc(mesh.vertexCount * 4 * sizeof(unsigned char));
  mesh.indices =
      (unsigned short *)malloc(mesh.triangleCount * 3 * sizeof(unsigned short));

  std::vector<float> heights(chunkSize * chunkSize);
  std::vector<float> moisture(chunkSize * chunkSize);
  std::vector<float> pathInfluence(chunkSize * chunkSize);

  for (int z = 0; z < chunkSize; z++) {
    for (int x = 0; x < chunkSize; x++) {
      float wx = cx * stride + x;
      float wz = cz * stride + z;
      int idx = z * chunkSize + x;

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
      float valleyNoise =
          db::perlin(wx * 0.001f + 500.0f, wz * 0.001f + 500.0f);
      if (valleyNoise < -0.25f) {
        height += (valleyNoise + 0.25f) * 3.0f; // Gentle broad valleys
      }

      heights[idx] = height + 3.0f;

      // Moisture
      moisture[idx] =
          0.5f +
          db::perlin(wx * 0.008f + 2000.0f, wz * 0.008f + 2000.0f) * 0.3f;
      if (heights[idx] < 2.5f) {
        moisture[idx] += 0.3f;
      }
      moisture[idx] = fminf(1.0f, fmaxf(0.0f, moisture[idx]));

      pathInfluence[idx] = getPathInfluence(wx, wz);
    }
  }

  // Reduced smoothing - the noise is already smooth at this scale
  const int smoothIterations = 4; // Less aggressive smoothing
  const int edgeMargin = 1;

  for (int iter = 0; iter < smoothIterations; iter++) {
    std::vector<float> newHeights = heights;
    for (int z = edgeMargin; z < chunkSize - edgeMargin; z++) {
      for (int x = edgeMargin; x < chunkSize - edgeMargin; x++) {
        int idx = z * chunkSize + x;

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
  for (int z = 0; z < chunkSize; z++) {
    for (int x = 0; x < chunkSize; x++) {
      int idx = z * chunkSize + x;
      float height = heights[idx];
      float m = moisture[idx];
      float pathVal = pathInfluence[idx];

      mesh.vertices[idx * 3] = (float)x;
      mesh.vertices[idx * 3 + 1] = height * 5.0f;
      mesh.vertices[idx * 3 + 2] = (float)z;

      mesh.texcoords[idx * 2] = (float)x / stride;
      mesh.texcoords[idx * 2 + 1] = (float)z / stride;

      unsigned char r, g, b;

      if (height < 1.5f) {
        r = 45 + (int)(m * 15.0f);
        g = 50 + (int)(m * 20.0f);
        b = 35 + (int)(m * 10.0f);
      } else if (height < 3.5f) {
        r = 60 + (int)(m * 15.0f);
        g = 65 + (int)(m * 20.0f);
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
        unsigned char pathR = 80;
        unsigned char pathG = 75;
        unsigned char pathB = 60;

        r = (unsigned char)(r * (1.0f - pathVal) + pathR * pathVal);
        g = (unsigned char)(g * (1.0f - pathVal) + pathG * pathVal);
        b = (unsigned char)(b * (1.0f - pathVal) + pathB * pathVal);
      }

      mesh.colors[idx * 4] = r;
      mesh.colors[idx * 4 + 1] = g;
      mesh.colors[idx * 4 + 2] = b;
      mesh.colors[idx * 4 + 3] = 255;
    }
  }

  // Compute normals
  for (int z = 0; z < chunkSize; z++) {
    for (int x = 0; x < chunkSize; x++) {
      int idx = z * chunkSize + x;
      float h = mesh.vertices[idx * 3 + 1];

      float hx1, hx2, hz1, hz2;

      if (x > 0)
        hx1 = mesh.vertices[(z * chunkSize + (x - 1)) * 3 + 1];
      else
        hx1 = h;

      if (x < chunkSize - 1)
        hx2 = mesh.vertices[(z * chunkSize + (x + 1)) * 3 + 1];
      else
        hx2 = h;

      if (z > 0)
        hz1 = mesh.vertices[((z - 1) * chunkSize + x) * 3 + 1];
      else
        hz1 = h;

      if (z < chunkSize - 1)
        hz2 = mesh.vertices[((z + 1) * chunkSize + x) * 3 + 1];
      else
        hz2 = h;

      float dx = (hx2 - hx1) / 2.0f;
      float dz = (hz2 - hz1) / 2.0f;

      Vector3 normal = {-dx, 2.0f, -dz};
      float len =
          sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
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
  for (int z = 0; z < stride; z++) {
    for (int x = 0; x < stride; x++) {
      int topLeft = z * chunkSize + x;
      int topRight = topLeft + 1;
      int bottomLeft = (z + 1) * chunkSize + x;
      int bottomRight = bottomLeft + 1;

      mesh.indices[indexCount++] = topLeft;
      mesh.indices[indexCount++] = bottomLeft;
      mesh.indices[indexCount++] = topRight;
      mesh.indices[indexCount++] = topRight;
      mesh.indices[indexCount++] = bottomLeft;
      mesh.indices[indexCount++] = bottomRight;
    }
  }

  UploadMesh(&mesh, false);
  Model model = LoadModelFromMesh(mesh);
  model.materials[0].shader = lightingShader;

  chunks[{cx, cz}] = {cx, cz, model, heights, moisture};
}

float getTerrainHeight(float wx, float wz) {
  // Determine which chunk this position is in
  const int stride = 31;
  int cx = (int)floor(wx / stride);
  int cz = (int)floor(wz / stride);

  // Check if chunk exists
  auto it = chunks.find({cx, cz});
  if (it == chunks.end()) {
    // Chunk not loaded, estimate height from noise
    float height = 0.0f;
    height += db::perlin(wx * 0.004f, wz * 0.004f) * 6.0f;
    height += db::perlin(wx * 0.01f + 100.0f, wz * 0.01f + 100.0f) * 2.5f;
    height += db::perlin(wx * 0.02f + 200.0f, wz * 0.02f + 200.0f) * 0.8f;
    height += db::perlin(wx * 0.05f + 300.0f, wz * 0.05f + 300.0f) * 0.3f;

    float valleyNoise = db::perlin(wx * 0.003f + 500.0f, wz * 0.003f + 500.0f);
    if (valleyNoise < -0.2f) {
      height += (valleyNoise + 0.2f) * 2.5f;
    }

    return (height + 3.0f) * 5.0f;
  }

  // Get local position within chunk
  float localX = wx - (cx * stride);
  float localZ = wz - (cz * stride);

  // Clamp to chunk bounds
  localX = fmaxf(0.0f, fminf(31.0f, localX));
  localZ = fmaxf(0.0f, fminf(31.0f, localZ));

  // Get the 4 surrounding vertices for bilinear interpolation
  int x0 = (int)floor(localX);
  int z0 = (int)floor(localZ);
  int x1 = fminf(x0 + 1, 31);
  int z1 = fminf(z0 + 1, 31);

  float fx = localX - x0;
  float fz = localZ - z0;

  // Get heights from chunk data
  const Chunk &chunk = it->second;
  float h00 = chunk.heights[z0 * 32 + x0];
  float h10 = chunk.heights[z0 * 32 + x1];
  float h01 = chunk.heights[z1 * 32 + x0];
  float h11 = chunk.heights[z1 * 32 + x1];

  // Bilinear interpolation
  float h0 = h00 * (1.0f - fx) + h10 * fx;
  float h1 = h01 * (1.0f - fx) + h11 * fx;
  float height = h0 * (1.0f - fz) + h1 * fz;

  return height * 5.0f; // Scale same as mesh
}
