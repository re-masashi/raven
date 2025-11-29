#include "game.h"
#include "raymath.h"
#include <cmath>
#include <iostream>

extern Camera camera;
extern float cameraYaw;
extern float cameraPitch;

// Player state
Vector3 playerVelocity = {0, 0, 0};
float playerHeight = 1.7f;
bool isGrounded = false;
float walkBobTimer = 0.0f;

// Walking parameters
const float walkSpeed = 4.0f;
const float sprintSpeed = 8.0f;
const float crouchSpeed = 2.0f;
const float gravity = 20.0f;
const float jumpForce = 8.0f;

// Head bob parameters
const float bobFrequency = 10.0f;
const float bobAmplitude = 0.08f;
const float bobAmplitudeSprint = 0.12f;

void UpdatePlayer(float deltaTime) {
  // Get forward/right vectors
  Vector3 forward = {sinf(cameraYaw) * cosf(cameraPitch),
                     0.0f, // Don't move up/down with look
                     cosf(cameraYaw) * cosf(cameraPitch)};
  forward = Vector3Normalize(forward);

  Vector3 right = {forward.z, 0.0f, -forward.x};

  // Determine speed
  float currentSpeed = walkSpeed;
  float currentBobAmp = bobAmplitude;

  if (IsKeyDown(KEY_LEFT_SHIFT)) {
    currentSpeed = sprintSpeed;
    currentBobAmp = bobAmplitudeSprint;
  } else if (IsKeyDown(KEY_LEFT_CONTROL)) {
    currentSpeed = crouchSpeed;
    currentBobAmp = bobAmplitude * 0.5f;
  }

  // Horizontal movement
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

  // Normalize movement
  float moveDirLength = sqrtf(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
  bool isMoving = moveDirLength > 0.01f;

  if (isMoving) {
    moveDir.x /= moveDirLength;
    moveDir.z /= moveDirLength;

    // Apply movement
    camera.position.x += moveDir.x * currentSpeed * deltaTime;
    camera.position.z += moveDir.z * currentSpeed * deltaTime;

    // Head bob
    walkBobTimer += deltaTime * bobFrequency;
    float bobOffset = sinf(walkBobTimer) * currentBobAmp;
    camera.position.y += bobOffset;
  } else {
    walkBobTimer = 0.0f;
  }

  // Gravity and ground detection
  float terrainHeight = getTerrainHeight(camera.position.x, camera.position.z);
  float desiredHeight = terrainHeight + playerHeight;

  // Apply gravity
  playerVelocity.y -= gravity * deltaTime;
  camera.position.y += playerVelocity.y * deltaTime;

  // Ground collision
  if (camera.position.y <= desiredHeight) {
    camera.position.y = desiredHeight;
    playerVelocity.y = 0.0f;
    isGrounded = true;
  } else {
    isGrounded = false;
  }

  // Jump
  if (IsKeyPressed(KEY_SPACE) && isGrounded) {
    playerVelocity.y = jumpForce;
    isGrounded = false;
  }

  // Noclip mode toggle (for debugging)
  static bool noclip = false;
  if (IsKeyPressed(KEY_N)) {
    noclip = !noclip;
    std::cout << "Noclip: " << (noclip ? "ON" : "OFF") << std::endl;
  }

  if (noclip) {
    // Free flight
    playerVelocity.y = 0;

    if (IsKeyDown(KEY_SPACE)) {
      camera.position.y += currentSpeed * deltaTime;
    }
    if (IsKeyDown(KEY_LEFT_ALT)) {
      camera.position.y -= currentSpeed * deltaTime;
    }
  }

  // Update camera target
  Vector3 lookForward = {sinf(cameraYaw) * cosf(cameraPitch), sinf(cameraPitch),
                         cosf(cameraYaw) * cosf(cameraPitch)};

  camera.target.x = camera.position.x + lookForward.x;
  camera.target.y = camera.position.y + lookForward.y;
  camera.target.z = camera.position.z + lookForward.z;
}
