#include "game.h"
#include "raymath.h"
#include <cmath>
#include <iostream>

extern Camera camera;
extern float cameraYaw;
extern float cameraPitch;

Vector3 playerVelocity = {0, 0, 0};
float playerHeight = 1.7f;
bool isGrounded = false;

const float walkSpeed = 4.0f;
const float sprintSpeed = 8.0f;
const float crouchSpeed = 2.0f;
const float gravity = 20.0f;
const float jumpForce = 8.0f;

float walkCycleTimer = 0.0f;
float landingTimer = 0.0f;
float velocitySmooth = 0.0f;
float breathTimer = 0.0f;
Vector3 headOffset = {0, 0, 0};
Vector3 headVelocity = {0, 0, 0};

const float stepFrequency = 1.8f;        // Steps per second at walk speed
const float stepHeight = 0.04f;          // Vertical head displacement per step
const float stepStrideRoll = 0.8f;       // Degrees of roll per step
const float stepStrideSway = 0.015f;     // Lateral head sway
const float landingImpact = 0.12f;       // Landing squat amount
const float landingRecoverySpeed = 5.0f; // How fast to recover from landing
const float breathFrequency = 0.25f;     // Breaths per second when idle
const float breathAmplitude = 0.003f;    // Subtle breathing motion
const float headDamping = 8.0f;          // Smoothness of head movement
const float velocitySmoothSpeed = 5.0f;  // How fast velocity changes smooth

// Perlin like noise function for natural variation
float smoothNoise(float x) {
  float intPart = floorf(x);
  float fractPart = x - intPart;
  float smoothed = fractPart * fractPart * (3.0f - 2.0f * fractPart);

  float v1 = sinf(intPart * 12.9898f) * 43758.5453f;
  float v2 = sinf((intPart + 1.0f) * 12.9898f) * 43758.5453f;
  v1 = v1 - floorf(v1);
  v2 = v2 - floorf(v2);

  return v1 + smoothed * (v2 - v1);
}

void UpdatePlayer(float deltaTime) {
  Vector3 forward = {sinf(cameraYaw), 0.0f, cosf(cameraYaw)};
  forward = Vector3Normalize(forward);
  Vector3 right = {forward.z, 0.0f, -forward.x};

  bool isSprinting = IsKeyDown(KEY_LEFT_SHIFT);
  bool isCrouching = IsKeyDown(KEY_LEFT_CONTROL);

  float currentSpeed = walkSpeed;
  if (isSprinting && !isCrouching)
    currentSpeed = sprintSpeed;
  else if (isCrouching)
    currentSpeed = crouchSpeed;

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
  float moveDirLength = sqrtf(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
  bool isMoving = moveDirLength > 0.01f;

  if (isMoving) {
    moveDir.x /= moveDirLength;
    moveDir.z /= moveDirLength;
    camera.position.x += moveDir.x * currentSpeed * deltaTime;
    camera.position.z += moveDir.z * currentSpeed * deltaTime;
  }

  // Smooth velocity for procedural effects
  float targetVelocity = isMoving ? currentSpeed : 0.0f;
  velocitySmooth +=
      (targetVelocity - velocitySmooth) * velocitySmoothSpeed * deltaTime;

  // === PROCEDURAL HEAD ANIMATION ===
  Vector3 targetHeadOffset = {0, 0, 0};

  if (velocitySmooth > 0.5f && isGrounded) {
    // Walking/Running cycle
    float speedRatio = currentSpeed / walkSpeed;
    walkCycleTimer += deltaTime * stepFrequency * speedRatio;

    float verticalBob =
        sinf(walkCycleTimer * 2.0f * PI) * stepHeight * speedRatio;

    float lateralSway = sinf(walkCycleTimer * PI) * stepStrideSway * speedRatio;

    float forwardBob = -sinf(walkCycleTimer * 2.0f * PI + PI * 0.5f) *
                       stepHeight * 0.3f * speedRatio;

    float noiseVariation =
        (smoothNoise(walkCycleTimer * 2.3f) - 0.5f) * 0.01f * speedRatio;

    targetHeadOffset.y = verticalBob + noiseVariation;
    targetHeadOffset.x = lateralSway * right.x;
    targetHeadOffset.z = lateralSway * right.z + forwardBob * forward.z;

  } else if (isGrounded) {
    // Idle breathing
    breathTimer += deltaTime * breathFrequency;
    float breathCycle = sinf(breathTimer * 2.0f * PI);
    targetHeadOffset.y = breathCycle * breathAmplitude;

    // Very subtle idle sway
    targetHeadOffset.x = sinf(breathTimer * PI * 0.7f) * breathAmplitude * 0.5f;
  }

  // Landing impact
  if (landingTimer > 0.0f) {
    float landingProgress = 1.0f - (landingTimer / 0.3f);
    float landingCurve = sinf(landingProgress * PI);
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

  float terrainHeight = getTerrainHeight(camera.position.x, camera.position.z);
  float desiredHeight = terrainHeight + playerHeight;

  bool wasGrounded = isGrounded;
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

  Vector3 lookForward = {sinf(cameraYaw) * cosf(cameraPitch), sinf(cameraPitch),
                         cosf(cameraYaw) * cosf(cameraPitch)};

  camera.target.x = camera.position.x + lookForward.x;
  camera.target.y = camera.position.y + lookForward.y;
  camera.target.z = camera.position.z + lookForward.z;
}
