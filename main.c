#include <raylib.h>

int main()
{
    const int screenWidth = 800;
    const int screenHeight = 450;
  
    InitWindow(screenWidth, screenHeight, "Pong");
    SetTargetFPS(60);
  
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Espero que não seja difícil de usar.", 150, 225, 25, DARKGRAY);
        EndDrawing();
    }
  
    CloseWindow();
    return 0;
}