#include "game.h"
#include "raymath.h"
#include <array>
#include <cmath>

struct Frustum {
  std::array<Vector4, 6> planes; // left, right, bottom, top, near, far
};

[[nodiscard]] Frustum ExtractFrustum(const Camera &camera) noexcept {
  const Matrix viewProj = MatrixMultiply(
      GetCameraMatrix(camera),
      MatrixPerspective(camera.fovy * DEG2RAD,
                        static_cast<float>(GetScreenWidth()) / static_cast<float>(GetScreenHeight()), 
                        0.1f, 1000.0f));

  Frustum frustum;
  // Left plane
  frustum.planes[0] = {viewProj.m3 + viewProj.m0, viewProj.m7 + viewProj.m4,
                       viewProj.m11 + viewProj.m8, viewProj.m15 + viewProj.m12};
  // Right plane
  frustum.planes[1] = {viewProj.m3 - viewProj.m0, viewProj.m7 - viewProj.m4,
                       viewProj.m11 - viewProj.m8, viewProj.m15 - viewProj.m12};
  // Bottom plane
  frustum.planes[2] = {viewProj.m3 + viewProj.m1, viewProj.m7 + viewProj.m5,
                       viewProj.m11 + viewProj.m9, viewProj.m15 + viewProj.m13};
  // Top plane
  frustum.planes[3] = {viewProj.m3 - viewProj.m1, viewProj.m7 - viewProj.m5,
                       viewProj.m11 - viewProj.m9, viewProj.m15 - viewProj.m13};
  // Near plane
  frustum.planes[4] = {viewProj.m3 + viewProj.m2, viewProj.m7 + viewProj.m6,
                       viewProj.m11 + viewProj.m10,
                       viewProj.m15 + viewProj.m14};
  // Far plane
  frustum.planes[5] = {viewProj.m3 - viewProj.m2, viewProj.m7 - viewProj.m6,
                       viewProj.m11 - viewProj.m10,
                       viewProj.m15 - viewProj.m14};

  // Normalize planes
  for (auto &plane : frustum.planes) {
    const float len = std::sqrt(plane.x * plane.x + plane.y * plane.y + plane.z * plane.z);
    plane.x /= len;
    plane.y /= len;
    plane.z /= len;
    plane.w /= len;
  }

  return frustum;
}

[[nodiscard]] bool IsChunkInFrustum(const Chunk &chunk, const Camera &camera) noexcept {
  const Frustum frustum = ExtractFrustum(camera);

  // Chunk bounding box (world space)
  const Vector3 chunkPos = {static_cast<float>(chunk.x * 31), 0.0f, static_cast<float>(chunk.z * 31)};
  constexpr float minY = 0.0f;
  constexpr float maxY = 40.0f;

  const std::array<Vector3, 8> corners = {{
      {chunkPos.x, minY, chunkPos.z},
      {chunkPos.x + 31, minY, chunkPos.z},
      {chunkPos.x, minY, chunkPos.z + 31},
      {chunkPos.x + 31, minY, chunkPos.z + 31},
      {chunkPos.x, maxY, chunkPos.z},
      {chunkPos.x + 31, maxY, chunkPos.z},
      {chunkPos.x, maxY, chunkPos.z + 31},
      {chunkPos.x + 31, maxY, chunkPos.z + 31}
  }};

  // Test against all 6 frustum planes
  for (const auto &plane : frustum.planes) {
    int out = 0;
    for (const auto &corner : corners) {
      const float dist = plane.x * corner.x + plane.y * corner.y + 
                        plane.z * corner.z + plane.w;
      if (dist < 0) {
        ++out;
      }
    }
    if (out == 8) {
      return false; // All corners outside this plane
    }
  }

  return true;
}
