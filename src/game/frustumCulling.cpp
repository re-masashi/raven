#include "game.h"
#include "raymath.h"

struct Frustum {
  Vector4 planes[6]; // left, right, bottom, top, near, far
};

Frustum ExtractFrustum(const Camera &camera) {
  Matrix viewProj = MatrixMultiply(
      GetCameraMatrix(camera),
      MatrixPerspective(camera.fovy * DEG2RAD,
                        (float)GetScreenWidth() / GetScreenHeight(), 0.1f,
                        1000.0f));

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
  for (int i = 0; i < 6; i++) {
    float len = sqrt(frustum.planes[i].x * frustum.planes[i].x +
                     frustum.planes[i].y * frustum.planes[i].y +
                     frustum.planes[i].z * frustum.planes[i].z);
    frustum.planes[i].x /= len;
    frustum.planes[i].y /= len;
    frustum.planes[i].z /= len;
    frustum.planes[i].w /= len;
  }

  return frustum;
}

bool IsChunkInFrustum(const Chunk &chunk, const Camera &camera) {
  Frustum frustum = ExtractFrustum(camera);

  // Chunk bounding box (world space)
  Vector3 chunkPos = {(float)(chunk.x * 31), 0.0f, (float)(chunk.z * 31)};
  float minY = 0.0f, maxY = 40.0f;

  Vector3 corners[8] = {{chunkPos.x, minY, chunkPos.z},
                        {chunkPos.x + 31, minY, chunkPos.z},
                        {chunkPos.x, minY, chunkPos.z + 31},
                        {chunkPos.x + 31, minY, chunkPos.z + 31},
                        {chunkPos.x, maxY, chunkPos.z},
                        {chunkPos.x + 31, maxY, chunkPos.z},
                        {chunkPos.x, maxY, chunkPos.z + 31},
                        {chunkPos.x + 31, maxY, chunkPos.z + 31}};

  // Test against all 6 frustum planes
  for (int i = 0; i < 6; i++) {
    int out = 0;
    for (int j = 0; j < 8; j++) {
      float dist = frustum.planes[i].x * corners[j].x +
                   frustum.planes[i].y * corners[j].y +
                   frustum.planes[i].z * corners[j].z + frustum.planes[i].w;
      if (dist < 0)
        out++;
    }
    if (out == 8)
      return false; // All corners outside this plane
  }

  return true;
}
