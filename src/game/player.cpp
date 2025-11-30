#include "game.h"
#include "raymath.h"
#include <cmath>
#include <iostream>
#include <numbers>

extern Camera camera;
extern float cameraYaw;
extern float cameraPitch;

constexpr float walkSpeed = 4.0f;
constexpr float sprintSpeed = 8.0f;
constexpr float crouchSpeed = 2.0f;
constexpr float gravity = 20.0f;
constexpr float jumpForce = 8.0f;

float walkCycleTimer = 0.0f;
float landingTimer = 0.0f;
float velocitySmooth = 0.0f;
float breathTimer = 0.0f;
Vector3 headOffset{0, 0, 0};
Vector3 headVelocity{0, 0, 0};

constexpr float stepFrequency = 1.8f;    // Steps per second at walk speed
constexpr float stepHeight = 0.04f;      // Vertical head displacement per step
constexpr float stepStrideRoll = 0.8f;   // Degrees of roll per step
constexpr float stepStrideSway = 0.015f; // Lateral head sway
constexpr float landingImpact = 0.2f;    // Landing squat amount
constexpr float landingRecoverySpeed = 5.0f; // How fast to recover from landing
constexpr float breathFrequency = 0.5f;      // Breaths per second when idle
constexpr float breathAmplitude = 0.0015f;   // Subtle breathing motion
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

void UpdatePlayer(float deltaTime) {
  Vector3 forward = {std::sin(cameraYaw), 0.0f, std::cos(cameraYaw)};
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
  const float moveDirLength =
      std::sqrt(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
  const bool isMoving = moveDirLength > 0.01f;

  if (isMoving) {
    moveDir.x /= moveDirLength;
    moveDir.z /= moveDirLength;
    camera.position.x += moveDir.x * currentSpeed * deltaTime;
    camera.position.z += moveDir.z * currentSpeed * deltaTime;
  }

  // Smooth velocity for procedural effects
  const float targetVelocity = isMoving ? currentSpeed : 0.0f;
  velocitySmooth +=
      (targetVelocity - velocitySmooth) * velocitySmoothSpeed * deltaTime;

  Vector3 targetHeadOffset{0, 0, 0};

  if (velocitySmooth > 0.5f && isGrounded) {
    // Walking/Running cycle
    const float speedRatio = currentSpeed / walkSpeed;
    walkCycleTimer += deltaTime * stepFrequency * speedRatio;

    using std::numbers::pi_v;
    const float verticalBob =
        std::sin(walkCycleTimer * 2.0f * pi_v<float>) * stepHeight * speedRatio;

    const float lateralSway =
        std::sin(walkCycleTimer * pi_v<float>) * stepStrideSway * speedRatio;

    const float forwardBob =
        -std::sin(walkCycleTimer * 2.0f * pi_v<float> + pi_v<float> * 0.5f) *
        stepHeight * 0.3f * speedRatio;

    const float noiseVariation =
        (smoothNoise(walkCycleTimer * 2.3f) - 0.5f) * 0.01f * speedRatio;

    targetHeadOffset.y = verticalBob + noiseVariation;
    targetHeadOffset.x = lateralSway * right.x;
    targetHeadOffset.z = lateralSway * right.z + forwardBob * forward.z;

  } else if (isGrounded) {
    // Idle breathing
    breathTimer += deltaTime * breathFrequency;
    using std::numbers::pi_v;
    const float breathCycle = std::sin(breathTimer * 2.0f * pi_v<float>);
    targetHeadOffset.y = breathCycle * breathAmplitude;

    // Very subtle idle sway
    targetHeadOffset.x =
        std::sin(breathTimer * pi_v<float> * 0.7f) * breathAmplitude * 0.5f;
  }

  // Landing impact
  if (landingTimer > 0.0f) {
    using std::numbers::pi_v;
    const float landingProgress = 1.0f - (landingTimer / 0.3f);
    const float landingCurve = std::sin(landingProgress * pi_v<float>);
    targetHeadOffset.y -= landingCurve * landingImpact;
    landingTimer -= deltaTime;
  }

  // Smooth head movement with spring damping
  headVelocity.x +=
      (targetHeadOffset.x - headOffset.x) * headDamping * deltaTime;
  headVelocity.y +=
      (targetHeadOffset.y - headOffset.y) * headDamping * deltaTime;
  headVelocity.z +=
      (targetHeadOffset.z - headOffset.z) * headDamping * deltaTime;

  // Apply damping
  headVelocity.x *= (1.0f - headDamping * deltaTime);
  headVelocity.y *= (1.0f - headDamping * deltaTime);
  headVelocity.z *= (1.0f - headDamping * deltaTime);

  headOffset.x += headVelocity.x * deltaTime;
  headOffset.y += headVelocity.y * deltaTime;
  headOffset.z += headVelocity.z * deltaTime;

  // Apply head offset to camera
  camera.position.x += headOffset.x;
  camera.position.y += headOffset.y;
  camera.position.z += headOffset.z;

  const float terrainHeight =
      getTerrainHeight(camera.position.x, camera.position.z);
  const float desiredHeight = terrainHeight + playerHeight;

  const bool wasGrounded = isGrounded;
  playerVelocity.y -= gravity * deltaTime;
  camera.position.y += playerVelocity.y * deltaTime;

  if (camera.position.y <= desiredHeight) {
    camera.position.y = desiredHeight;

    // Landing impact detection
    if (!wasGrounded && playerVelocity.y < -5.0f) {
      landingTimer = 0.3f;
    }

    playerVelocity.y = 0.0f;
    isGrounded = true;
  } else {
    isGrounded = false;
  }

  // Jump
  if (IsKeyPressed(KEY_SPACE) && isGrounded) {
    playerVelocity.y = jumpForce;
    isGrounded = false;
    walkCycleTimer = 0.0f; // Reset walk cycle on jump
  }

  // Noclip mode
  static bool noclip = false;
  if (IsKeyPressed(KEY_N)) {
    noclip = !noclip;
    std::cout << "Noclip: " << (noclip ? "ON" : "OFF") << std::endl;
  }

  if (noclip) {
    playerVelocity.y = 0;
    if (IsKeyDown(KEY_SPACE)) {
      camera.position.y += currentSpeed * deltaTime;
    }
    if (IsKeyDown(KEY_LEFT_ALT)) {
      camera.position.y -= currentSpeed * deltaTime;
    }
  }

  const Vector3 lookForward = {std::sin(cameraYaw) * std::cos(cameraPitch),
                               std::sin(cameraPitch),
                               std::cos(cameraYaw) * std::cos(cameraPitch)};

  camera.target.x = camera.position.x + lookForward.x;
  camera.target.y = camera.position.y + lookForward.y;
  camera.target.z = camera.position.z + lookForward.z;
}
