#include "game.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <numbers>
#include <random>
#include <vector>

void GenerateStars(GameContext& ctx) {
  ctx.stars.clear();
  ctx.stars.reserve(400);
  
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dist(0.0f, 1.0f);

  using std::numbers::pi_v;
  constexpr float radius = 500.0f;
  
  for (int i = 0; i < 400; ++i) {
    Star star;

    const float theta = dist(gen) * 2.0f * pi_v<float>;
    const float phi = dist(gen) * pi_v<float>;

    star.position.x = radius * std::sin(phi) * std::cos(theta);
    star.position.y = radius * std::cos(phi);
    star.position.z = radius * std::sin(phi) * std::sin(theta);

    star.brightness = 0.3f + dist(gen) * 0.7f;
    star.size = 1.0f + dist(gen) * 2.0f;

    ctx.stars.push_back(star);
  }

  std::cout << "Generated " << ctx.stars.size() << " stars" << std::endl;
}

void DrawSky(const GameContext& ctx, const Camera& camera) {
  constexpr Color fogColor = {115, 102, 97, 255};
  constexpr Color skyColorTop = {20, 15, 18, 255};
  constexpr Color skyColorHorizon = fogColor;

  const Vector3 skyCenter = camera.position;

  constexpr int segments = 16;
  constexpr float skyRadius = 400.0f;

  using std::numbers::pi_v;
  
  for (int i = 0; i < segments; ++i) {
    const float angle1 = static_cast<float>(i) / segments * pi_v<float> / 2.0f;
    const float angle2 = static_cast<float>(i + 1) / segments * pi_v<float> / 2.0f;

    const float y1 = skyRadius * std::cos(angle1);
    const float y2 = skyRadius * std::cos(angle2);
    const float r1 = skyRadius * std::sin(angle1);
    const float r2 = skyRadius * std::sin(angle2);

    const float t1 = static_cast<float>(i) / segments;
    const float t2 = static_cast<float>(i + 1) / segments;

    const Color color1 = {
        static_cast<unsigned char>(skyColorTop.r * (1 - t1) + skyColorHorizon.r * t1),
        static_cast<unsigned char>(skyColorTop.g * (1 - t1) + skyColorHorizon.g * t1),
        static_cast<unsigned char>(skyColorTop.b * (1 - t1) + skyColorHorizon.b * t1),
        255};

    const Color color2 = {
        static_cast<unsigned char>(skyColorTop.r * (1 - t2) + skyColorHorizon.r * t2),
        static_cast<unsigned char>(skyColorTop.g * (1 - t2) + skyColorHorizon.g * t2),
        static_cast<unsigned char>(skyColorTop.b * (1 - t2) + skyColorHorizon.b * t2),
        255};

    for (int j = 0; j < 32; ++j) {
      const float theta1 = static_cast<float>(j) / 32 * 2.0f * pi_v<float>;
      const float theta2 = static_cast<float>(j + 1) / 32 * 2.0f * pi_v<float>;

      const Vector3 p1 = {skyCenter.x + r1 * std::cos(theta1), skyCenter.y + y1,
                          skyCenter.z + r1 * std::sin(theta1)};
      const Vector3 p2 = {skyCenter.x + r1 * std::cos(theta2), skyCenter.y + y1,
                          skyCenter.z + r1 * std::sin(theta2)};
      const Vector3 p3 = {skyCenter.x + r2 * std::cos(theta2), skyCenter.y + y2,
                          skyCenter.z + r2 * std::sin(theta2)};
      const Vector3 p4 = {skyCenter.x + r2 * std::cos(theta1), skyCenter.y + y2,
                          skyCenter.z + r2 * std::sin(theta1)};

      DrawTriangle3D(p1, p2, p3, color1);
      DrawTriangle3D(p1, p3, p4, color2);
    }
  }

  // Draw stars with fog fade
  for (const auto &star : ctx.stars) {
    const Vector3 starPos = {camera.position.x + star.position.x,
                             camera.position.y + star.position.y,
                             camera.position.z + star.position.z};

    const float dist = Vector3Distance(camera.position, starPos);
    constexpr float fogStart = 15.0f;
    const float fogFactor = std::clamp(1.0f - std::exp(-0.04f * std::max(0.0f, dist - fogStart)),
                                       0.0f, 1.0f);

    const float starAlpha = star.brightness * (1.0f - fogFactor);

    if (starAlpha > 0.1f) {
      const Color starColor = {
          static_cast<unsigned char>(255 * starAlpha), 
          static_cast<unsigned char>(240 * starAlpha),
          static_cast<unsigned char>(220 * starAlpha), 
          static_cast<unsigned char>(255 * starAlpha)};
      DrawSphere(starPos, star.size, starColor);
    }
  }
}

void CleanupSky(GameContext& ctx) { ctx.stars.clear(); }
