#include "game.h"
#include "raymath.h"
#include <cmath>
#include <format>

extern Camera camera;
extern Font font;

void ApplyWorldBoundaries(float deltaTime) {
  // Calculate distance from world center (only XZ plane, ignore Y)
  const float dx = camera.position.x - WORLD_CENTER.x;
  const float dz = camera.position.z - WORLD_CENTER.z;
  const float distanceFromCenter = std::sqrt(dx * dx + dz * dz);

  // No boundary effects if within soft boundary
  if (distanceFromCenter <= SOFT_BOUNDARY_START) {
    return;
  }

  // Calculate direction vector pointing toward center
  const Vector3 directionToCenter = {
      (WORLD_CENTER.x - camera.position.x) / distanceFromCenter, 0.0f,
      (WORLD_CENTER.z - camera.position.z) / distanceFromCenter};

  // Gentle push when beyond soft boundary but within hard boundary
  if (distanceFromCenter > SOFT_BOUNDARY_START &&
      distanceFromCenter <= HARD_BOUNDARY_START) {
    // Interpolate push strength from 0 to moderate
    const float pushRatio =
        (distanceFromCenter - SOFT_BOUNDARY_START) /
        (HARD_BOUNDARY_START - SOFT_BOUNDARY_START);
    const float pushStrength = pushRatio * 5.0f; // Max 5 units/sec gentle push

    camera.position.x += directionToCenter.x * pushStrength * deltaTime;
    camera.position.z += directionToCenter.z * pushStrength * deltaTime;
  }
  // Strong push when beyond hard boundary
  else if (distanceFromCenter > HARD_BOUNDARY_START) {
    const float excessDistance = distanceFromCenter - HARD_BOUNDARY_START;
    const float pushStrength =
        20.0f + excessDistance * 2.0f; // Strong increasing push

    camera.position.x += directionToCenter.x * pushStrength * deltaTime;
    camera.position.z += directionToCenter.z * pushStrength * deltaTime;
  }
}

void DrawBoundaryWarning() {
  // Calculate distance from world center
  const float dx = camera.position.x - WORLD_CENTER.x;
  const float dz = camera.position.z - WORLD_CENTER.z;
  const float distanceFromCenter = std::sqrt(dx * dx + dz * dz);

  // Only show warning beyond world radius
  if (distanceFromCenter <= WORLD_RADIUS) {
    return;
  }

  const int screenWidth = GetScreenWidth();
  const int screenHeight = GetScreenHeight();

  // Calculate warning intensity based on distance
  float warningIntensity = 0.0f;
  const char *warningText = nullptr;
  Color warningColor = YELLOW;

  if (distanceFromCenter > WORLD_RADIUS &&
      distanceFromCenter <= HARD_BOUNDARY_START) {
    // Soft boundary warning
    warningIntensity =
        (distanceFromCenter - WORLD_RADIUS) / (HARD_BOUNDARY_START - WORLD_RADIUS);
    warningText = "You are approaching the edge of the world...";
    warningColor = ColorAlpha(YELLOW, warningIntensity * 0.8f);
  } else if (distanceFromCenter > HARD_BOUNDARY_START) {
    // Hard boundary warning - pulsing
    const float pulseSpeed = 3.0f;
    const float pulse = (std::sin(GetTime() * pulseSpeed) + 1.0f) * 0.5f;
    warningIntensity = 0.5f + pulse * 0.5f; // Pulse between 0.5 and 1.0
    warningText = "!!! TURN BACK - WORLD BOUNDARY !!!";
    warningColor = ColorAlpha(RED, warningIntensity);
  }

  if (warningText != nullptr) {
    // Draw warning text at top of screen
    const Vector2 textSize = MeasureTextEx(font, warningText, 24, 1.0f);
    const Vector2 textPos = {screenWidth / 2.0f - textSize.x / 2.0f, 100.0f};

    // Draw shadow for better visibility
    DrawTextEx(font, warningText, {textPos.x + 2, textPos.y + 2}, 24, 1.0f,
               ColorAlpha(BLACK, warningIntensity * 0.5f));
    DrawTextEx(font, warningText, textPos, 24, 1.0f, warningColor);

    // Draw distance indicator
    const float distanceToEdge = distanceFromCenter - WORLD_RADIUS;
    const std::string distText =
        std::format("Distance beyond boundary: {:.1f} units", distanceToEdge);
    const Vector2 distTextSize = MeasureTextEx(font, distText.c_str(), 18, 1.0f);
    const Vector2 distTextPos = {screenWidth / 2.0f - distTextSize.x / 2.0f,
                                 130.0f};

    DrawTextEx(font, distText.c_str(), {distTextPos.x + 1, distTextPos.y + 1},
               18, 1.0f, ColorAlpha(BLACK, warningIntensity * 0.5f));
    DrawTextEx(font, distText.c_str(), distTextPos, 18, 1.0f,
               ColorAlpha(WHITE, warningIntensity * 0.7f));
  }
}
