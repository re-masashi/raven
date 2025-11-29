// src/game/sky.cpp
#include "game.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

struct Star {
  Vector3 position;
  float brightness;
  float size;
};

std::vector<Star> stars;

void GenerateStars() {
  stars.clear();
  for (int i = 0; i < 400; i++) {
    Star star;

    float theta = ((float)rand() / RAND_MAX) * 2.0f * PI;
    float phi = ((float)rand() / RAND_MAX) * PI;
    float radius = 500.0f;

    star.position.x = radius * sin(phi) * cos(theta);
    star.position.y = radius * cos(phi);
    star.position.z = radius * sin(phi) * sin(theta);

    star.brightness = 0.3f + ((float)rand() / RAND_MAX) * 0.7f;
    star.size = 1.0f + ((float)rand() / RAND_MAX) * 2.0f;

    stars.push_back(star);
  }

  std::cout << "Generated " << stars.size() << " stars" << std::endl;
}

void DrawSky(Camera camera) {
  Color fogColor = (Color){115, 102, 97, 255};
  Color skyColorTop = (Color){20, 15, 18, 255};
  Color skyColorHorizon = fogColor;

  Vector3 skyCenter = camera.position;

  const int segments = 16;
  const float skyRadius = 400.0f;

  for (int i = 0; i < segments; i++) {
    float angle1 = (float)i / segments * PI / 2.0f;
    float angle2 = (float)(i + 1) / segments * PI / 2.0f;

    float y1 = skyRadius * cos(angle1);
    float y2 = skyRadius * cos(angle2);
    float r1 = skyRadius * sin(angle1);
    float r2 = skyRadius * sin(angle2);

    float t1 = (float)i / segments;
    float t2 = (float)(i + 1) / segments;

    Color color1 = {
        (unsigned char)(skyColorTop.r * (1 - t1) + skyColorHorizon.r * t1),
        (unsigned char)(skyColorTop.g * (1 - t1) + skyColorHorizon.g * t1),
        (unsigned char)(skyColorTop.b * (1 - t1) + skyColorHorizon.b * t1),
        255};

    Color color2 = {
        (unsigned char)(skyColorTop.r * (1 - t2) + skyColorHorizon.r * t2),
        (unsigned char)(skyColorTop.g * (1 - t2) + skyColorHorizon.g * t2),
        (unsigned char)(skyColorTop.b * (1 - t2) + skyColorHorizon.b * t2),
        255};

    for (int j = 0; j < 32; j++) {
      float theta1 = (float)j / 32 * 2.0f * PI;
      float theta2 = (float)(j + 1) / 32 * 2.0f * PI;

      Vector3 p1 = {skyCenter.x + r1 * cos(theta1), skyCenter.y + y1,
                    skyCenter.z + r1 * sin(theta1)};
      Vector3 p2 = {skyCenter.x + r1 * cos(theta2), skyCenter.y + y1,
                    skyCenter.z + r1 * sin(theta2)};
      Vector3 p3 = {skyCenter.x + r2 * cos(theta2), skyCenter.y + y2,
                    skyCenter.z + r2 * sin(theta2)};
      Vector3 p4 = {skyCenter.x + r2 * cos(theta1), skyCenter.y + y2,
                    skyCenter.z + r2 * sin(theta1)};

      DrawTriangle3D(p1, p2, p3, color1);
      DrawTriangle3D(p1, p3, p4, color2);
    }
  }

  // Draw stars with fog fade
  for (const auto &star : stars) {
    Vector3 starPos = {camera.position.x + star.position.x,
                       camera.position.y + star.position.y,
                       camera.position.z + star.position.z};

    float dist = Vector3Distance(camera.position, starPos);
    float fogStart = 15.0f;
    float fogFactor = 1.0f - exp(-0.04f * std::max(0.0f, dist - fogStart));
    fogFactor = std::clamp(fogFactor, 0.0f, 1.0f);

    float starAlpha = star.brightness * (1.0f - fogFactor);

    if (starAlpha > 0.1f) {
      Color starColor = {
          (unsigned char)(255 * starAlpha), (unsigned char)(240 * starAlpha),
          (unsigned char)(220 * starAlpha), (unsigned char)(255 * starAlpha)};
      DrawSphere(starPos, star.size, starColor);
    }
  }
}

void CleanupSky() { stars.clear(); }
