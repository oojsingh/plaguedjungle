#pragma once

#include "GameState.h"
#include "raylib.h"
#include <string>
#include <vector>

class MenuState : public GameState {
public:
    MenuState();
    ~MenuState() override = default;

    void Init(GameStateManager* manager) override;
    void Cleanup() override;

    void Update(float dt) override;
    void Draw() override;

private:
    struct LevelOption {
        std::string name;
        std::string description;
        Color accent;
    };

    std::vector<LevelOption> m_levels;
    int m_selectedLevel = 0;

    void StartSelectedLevel();
    void UpdateSelectionFromMouse();
    void DrawLevelCard(const LevelOption& level, Rectangle bounds, bool selected) const;
};
