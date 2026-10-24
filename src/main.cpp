#include "raylib.h"
#include "ResourceManager.h"
#include "GameStateManager.h"
#include "PlayState.h"
#include <algorithm>

int main() {
    // Window configuration
    const int screenWidth = 960;
    const int screenHeight = 600;

    // Raylib window initialization
    InitWindow(screenWidth, screenHeight, "Canopy Snap");
    SetTargetFPS(60);

    // Initialize asset system & generate pixel-art textures if not already generated
    ResourceManager::GetInstance().InitializeDefaultAssets();

    // Create and initialize Game State Manager with PlayState
    GameStateManager stateManager;
    stateManager.ChangeState(std::make_unique<PlayState>());

    // Main game loop
    while (!WindowShouldClose() && stateManager.IsRunning()) {
        // Delta time with clamp to prevent tunneling during lag spikes or window drags
        float dt = GetFrameTime();
        dt = std::min(dt, 0.05f);

        // 1. Update active game state logic & physics
        stateManager.Update(dt);

        // 2. Render active game state
        BeginDrawing();
        ClearBackground(RAYWHITE);

        stateManager.Draw();

        EndDrawing();
    }

    // Explicit cleanup
    ResourceManager::GetInstance().UnloadAll();
    CloseWindow();

    return 0;
}
