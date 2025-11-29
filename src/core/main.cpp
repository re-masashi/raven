#include "../game/game.h"
#include "raylib.h"

int main() {
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(1080, 720, "The Raven");
  SetWindowState(FLAG_WINDOW_RESIZABLE);
  // SetTargetFPS(60);

  InitGame();

  while (!WindowShouldClose()) {
    UpdateGame();
    BeginDrawing();
    ClearBackground(Color{15, 15, 20, 255});
    DrawGame();
    EndDrawing();
  }

  UnloadGame();
  CloseWindow();
  return 0;
}
