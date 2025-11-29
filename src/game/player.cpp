#include "game.h"
#include "raymath.h"
#include <cmath>
#include <iostream>
#include <numbers>

constexpr float walkSpeed = 4.0f;
constexpr float sprintSpeed = 8.0f;
constexpr float crouchSpeed = 2.0f;
constexpr float gravity = 20.0f;
constexpr float jumpForce = 8.0f;

constexpr float stepFrequency = 1.8f;        // Steps per second at walk speed
constexpr float stepHeight = 0.04f;          // Vertical head displacement per step
constexpr float stepStrideRoll = 0.8f;       // Degrees of roll per step
constexpr float stepStrideSway = 0.015f;     // Lateral head sway
constexpr float landingImpact = 0.12f;       // Landing squat amount
constexpr float landingRecoverySpeed = 5.0f; // How fast to recover from landing
constexpr float breathFrequency = 0.25f;     // Breaths per second when idle
constexpr float breathAmplitude = 0.003f;    // Subtle breathing motion
constexpr float headDamping = 8.0f;          // Smoothness of head movement
constexpr float velocitySmoothSpeed = 5.0f;  // How fast velocity changes smooth

// Perlin like noise function for natural variation
[[nodiscard]] constexpr float smoothNoise(float x) noexcept {
  const float intPart = std::floor(x);
  const float fractPart = x - intPart;
  const float smoothed = fractPart * fractPart * (3.0f - 2.0f * fractPart);

  float v1 = std::sin(intPart * 12.9898f) * 43758.5453f;
  float v2 = std::sin((intPart + 1.0f) * 12.9898f) * 43758.5453f;
  v1 = v1 - std::floor(v1);
  v2 = v2 - std::floor(v2);

  return v1 + smoothed * (v2 - v1);
}

void UpdatePlayer(GameContext& ctx, float deltaTime) {
  Vector3 forward = {std::sin(ctx.cameraYaw), 0.0f, std::cos(ctx.cameraYaw)};
  forward = Vector3Normalize(forward);
  const Vector3 right = {forward.z, 0.0f, -forward.x};

  const bool isSprinting = IsKeyDown(KEY_LEFT_SHIFT);
  const bool isCrouching = IsKeyDown(KEY_LEFT_CONTROL);

  float currentSpeed = walkSpeed;
  if (isSprinting && !isCrouching) {
    currentSpeed = sprintSpeed;
  } else if (isCrouching) {
    currentSpeed = crouchSpeed;
  }

  // Horizontal movement input
  Vector3 moveDir = {0, 0, 0};
  if (IsKeyDown(KEY_W)) {
    moveDir.x += forward.x;
    moveDir.z += forward.z;
  }
  if (IsKeyDown(KEY_S)) {
    moveDir.x -= forward.x;
    moveDir.z -= forward.z;
  }
  if (IsKeyDown(KEY_D)) {
    moveDir.x += right.x;
    moveDir.z += right.z;
  }
  if (IsKeyDown(KEY_A)) {
    moveDir.x -= right.x;
    moveDir.z -= right.z;
  }

  // Normalize and apply movement
  const float moveDirLength = std::sqrt(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
  const bool isMoving = moveDirLength > 0.01f;

  if (isMoving) {
    moveDir.x /= moveDirLength;
    moveDir.z /= moveDirLength;
    ctx.camera.position.x += moveDir.x * currentSpeed * deltaTime;
    ctx.camera.position.z += moveDir.z * currentSpeed * deltaTime;
  }

  // Smooth velocity for procedural effects
  const float targetVelocity = isMoving ? currentSpeed : 0.0f;
  ctx.playerAnim.velocitySmooth += (targetVelocity - ctx.playerAnim.velocitySmooth) * velocitySmoothSpeed * deltaTime;

  // === PROCEDURAL HEAD ANIMATION ===
  Vector3 targetHeadOffset{0, 0, 0};

  if (ctx.playerAnim.velocitySmooth > 0.5f && ctx.isGrounded) {
    // Walking/Running cycle
    const float speedRatio = currentSpeed / walkSpeed;
    ctx.playerAnim.walkCycleTimer += deltaTime * stepFrequency * speedRatio;

    using std::numbers::pi_v;
    const float verticalBob = std::sin(ctx.playerAnim.walkCycleTimer * 2.0f * pi_v<float>) * stepHeight * speedRatio;

    const float lateralSway = std::sin(ctx.playerAnim.walkCycleTimer * pi_v<float>) * stepStrideSway * speedRatio;

    const float forwardBob = -std::sin(ctx.playerAnim.walkCycleTimer * 2.0f * pi_v<float> + pi_v<float> * 0.5f) *
                             stepHeight * 0.3f * speedRatio;

    const float noiseVariation = (smoothNoise(ctx.playerAnim.walkCycleTimer * 2.3f) - 0.5f) * 0.01f * speedRatio;

    targetHeadOffset.y = verticalBob + noiseVariation;
    targetHeadOffset.x = lateralSway * right.x;
    targetHeadOffset.z = lateralSway * right.z + forwardBob * forward.z;

  } else if (ctx.isGrounded) {
    // Idle breathing
    ctx.playerAnim.breathTimer += deltaTime * breathFrequency;
    using std::numbers::pi_v;
    const float breathCycle = std::sin(ctx.playerAnim.breathTimer * 2.0f * pi_v<float>);
    targetHeadOffset.y = breathCycle * breathAmplitude;

    // Very subtle idle sway
    targetHeadOffset.x = std::sin(ctx.playerAnim.breathTimer * pi_v<float> * 0.7f) * breathAmplitude * 0.5f;
  }

  // Landing impact
  if (ctx.playerAnim.landingTimer > 0.0f) {
    using std::numbers::pi_v;
    const float landingProgress = 1.0f - (ctx.playerAnim.landingTimer / 0.3f);
    const float landingCurve = std::sin(landingProgress * pi_v<float>);
    targetHeadOffset.y -= landingCurve * landingImpact;
    ctx.playerAnim.landingTimer -= deltaTime;
  }

  // Smooth head movement with spring damping
  ctx.playerAnim.headVelocity.x +=
      (targetHeadOffset.x - ctx.playerAnim.headOffset.x) * headDamping * deltaTime;
  ctx.playerAnim.headVelocity.y +=
      (targetHeadOffset.y - ctx.playerAnim.headOffset.y) * headDamping * deltaTime;
  ctx.playerAnim.headVelocity.z +=
      (targetHeadOffset.z - ctx.playerAnim.headOffset.z) * headDamping * deltaTime;

  // Apply damping
  ctx.playerAnim.headVelocity.x *= (1.0f - headDamping * deltaTime);
  ctx.playerAnim.headVelocity.y *= (1.0f - headDamping * deltaTime);
  ctx.playerAnim.headVelocity.z *= (1.0f - headDamping * deltaTime);

  ctx.playerAnim.headOffset.x += ctx.playerAnim.headVelocity.x * deltaTime;
  ctx.playerAnim.headOffset.y += ctx.playerAnim.headVelocity.y * deltaTime;
  ctx.playerAnim.headOffset.z += ctx.playerAnim.headVelocity.z * deltaTime;

  // Apply head offset to camera
  ctx.camera.position.x += ctx.playerAnim.headOffset.x;
  ctx.camera.position.y += ctx.playerAnim.headOffset.y;
  ctx.camera.position.z += ctx.playerAnim.headOffset.z;

  const float terrainHeight = getTerrainHeight(ctx, ctx.camera.position.x, ctx.camera.position.z);
  const float desiredHeight = terrainHeight + ctx.playerHeight;

  const bool wasGrounded = ctx.isGrounded;
  ctx.playerVelocity.y -= gravity * deltaTime;
  ctx.camera.position.y += ctx.playerVelocity.y * deltaTime;

  if (ctx.camera.position.y <= desiredHeight) {
    ctx.camera.position.y = desiredHeight;

    // Landing impact detection
    if (!wasGrounded && ctx.playerVelocity.y < -5.0f) {
      ctx.playerAnim.landingTimer = 0.3f;
    }

    ctx.playerVelocity.y = 0.0f;
    ctx.isGrounded = true;
  } else {
    ctx.isGrounded = false;
  }

  // Jump
  if (IsKeyPressed(KEY_SPACE) && ctx.isGrounded) {
    ctx.playerVelocity.y = jumpForce;
    ctx.isGrounded = false;
    ctx.playerAnim.walkCycleTimer = 0.0f; // Reset walk cycle on jump
  }

  // Noclip mode
  static bool noclip = false;
  if (IsKeyPressed(KEY_N)) {
    noclip = !noclip;
    std::cout << "Noclip: " << (noclip ? "ON" : "OFF") << std::endl;
  }

  if (noclip) {
    ctx.playerVelocity.y = 0;
    if (IsKeyDown(KEY_SPACE)) {
      ctx.camera.position.y += currentSpeed * deltaTime;
    }
    if (IsKeyDown(KEY_LEFT_ALT)) {
      ctx.camera.position.y -= currentSpeed * deltaTime;
    }
  }

  const Vector3 lookForward = {std::sin(ctx.cameraYaw) * std::cos(ctx.cameraPitch), 
                               std::sin(ctx.cameraPitch),
                               std::cos(ctx.cameraYaw) * std::cos(ctx.cameraPitch)};

  ctx.camera.target.x = ctx.camera.position.x + lookForward.x;
  ctx.camera.target.y = ctx.camera.position.y + lookForward.y;
  ctx.camera.target.z = ctx.camera.position.z + lookForward.z;
}
