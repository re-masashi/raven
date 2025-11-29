#include "../game/game.h"
#include "raylib.h"

int main() {
  InitWindow(1080, 720, "The Raven");
  SetWindowState(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(60);

  InitGame(); // Initialize game logic

  while (!WindowShouldClose()) {
    UpdateGame(); // Update game logic
    BeginDrawing();
    ClearBackground((Color){25, 15, 20, 255});
    DrawGame(); // Draw game logic
    EndDrawing();
  }

  UnloadGame(); // Unload game logic
  CloseWindow();
  return 0;
}
