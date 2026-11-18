#include "MenuState.h"
#include "GameStateManager.h"
#include "PlayState.h"
#include <algorithm>

MenuState::MenuState() {
    m_levels = {
        {"1. High Roost Run", "Learn vines, momentum, and pounces.", Color{ 42, 204, 113, 255 }},
        {"2. Bramble Sprint", "Fast gaps, sharp turns, tight hazards.", Color{ 52, 152, 219, 255 }},
        {"3. Sanctum Gauntlet", "The final climb to the sanctuary.", Color{ 255, 140, 0, 255 }},
        {"4. Misdirection Mire", "Fake anchors and slick, oiled bark.", Color{ 180, 90, 150, 255 }},
        {"5. Panic Pendulum", "Time your way past swinging thorn logs.", Color{ 255, 90, 60, 255 }},
        {"6. Sunk Cost Spire", "Climb between rising rot and debris.", Color{ 235, 190, 70, 255 }},
        {"7. Phantom Branch", "Platforms vanish after two seconds.", Color{ 130, 170, 255, 255 }}
    };
}

void MenuState::Init(GameStateManager* manager) {
    m_manager = manager;
    m_selectedLevel = 0;
}

void MenuState::Cleanup() {
    m_levels.clear();
}

void MenuState::Update(float dt) {
    (void)dt;

    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        m_selectedLevel = std::min(m_selectedLevel + 3, static_cast<int>(m_levels.size()) - 1);
    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        m_selectedLevel = std::max(m_selectedLevel - 3, 0);
    }
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
        m_selectedLevel = std::min(m_selectedLevel + 1, static_cast<int>(m_levels.size()) - 1);
    }
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
        m_selectedLevel = std::max(m_selectedLevel - 1, 0);
    }
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        StartSelectedLevel();
    }

    UpdateSelectionFromMouse();

    if (IsKeyPressed(KEY_ESCAPE)) {
        if (m_manager) {
            m_manager->Quit();
        }
    }
}

void MenuState::UpdateSelectionFromMouse() {
    if (m_levels.empty()) return;

    for (int i = 0; i < static_cast<int>(m_levels.size()); ++i) {
        int x = 100 + (i % 3) * 260;
        int y = 175 + (i / 3) * 110;
        Rectangle card = { static_cast<float>(x), static_cast<float>(y), 240.0f, 90.0f };
        if (CheckCollisionPointRec(GetMousePosition(), card) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            m_selectedLevel = i;
            StartSelectedLevel();
            return;
        }
    }
}

void MenuState::StartSelectedLevel() {
    if (!m_manager) return;
    m_manager->ChangeState(std::make_unique<PlayState>(m_selectedLevel));
}

void MenuState::DrawLevelCard(const LevelOption& level, Rectangle bounds, bool selected) const {
    Color fill = selected ? Color{ 18, 28, 40, 255 } : Color{ 12, 18, 26, 220 };
    Color border = selected ? level.accent : Color{ 65, 78, 98, 255 };
    DrawRectangleRec(bounds, fill);
    DrawRectangleLinesEx(bounds, selected ? 3.0f : 2.0f, border);

    DrawText(level.name.c_str(), static_cast<int>(bounds.x + 12), static_cast<int>(bounds.y + 10), 16, WHITE);
    DrawText(level.description.c_str(), static_cast<int>(bounds.x + 12), static_cast<int>(bounds.y + 38), 11, Color{ 200, 210, 220, 255 });
    DrawText(selected ? "ENTER / CLICK TO PLAY" : "CLICK TO SELECT", static_cast<int>(bounds.x + 12), static_cast<int>(bounds.y + 63), 10, level.accent);
}

void MenuState::Draw() {
    ClearBackground(Color{ 8, 12, 20, 255 });

    DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), Color{ 10, 18, 30, 255 }, Color{ 14, 22, 36, 255 });

    DrawText("CANOPY SNAP", 100, 50, 36, Color{ 0, 255, 220, 255 });
    DrawText("Choose a level to begin your jungle escape.", 100, 95, 18, Color{ 205, 220, 235, 255 });

    DrawText("[WASD / Arrow Keys] select   [Enter/Space] start   [Esc] quit", 100, 130, 12, Color{ 140, 170, 200, 255 });

    const int cardWidth = 240;
    const int cardHeight = 90;
    const int gap = 20;
    const int startX = 100;
    const int startY = 175;

    for (int i = 0; i < static_cast<int>(m_levels.size()); ++i) {
        Rectangle card = {
            static_cast<float>(startX + (i % 3) * (cardWidth + gap)),
            static_cast<float>(startY + (i / 3) * (cardHeight + gap)),
            static_cast<float>(cardWidth),
            static_cast<float>(cardHeight)
        };
        DrawLevelCard(m_levels[i], card, i == m_selectedLevel);
    }

    DrawRectangle(90, 520, 780, 38, Color{ 14, 18, 26, 200 });
    DrawText("Controls: [Space/Up/W] jump, hold [Space/LMB] to swing, [E/RMB] tail snap, [Q/F] pounce, [R] reset", 110, 532, 11, Color{ 180, 200, 220, 255 });
}
