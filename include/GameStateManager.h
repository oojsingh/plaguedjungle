#pragma once

#include "GameState.h"
#include <vector>
#include <memory>

class GameStateManager {
public:
    GameStateManager();
    ~GameStateManager();

    void ChangeState(std::unique_ptr<GameState> state);
    void PushState(std::unique_ptr<GameState> state);
    void PopState();

    void Update(float dt);
    void Draw();

    bool IsRunning() const { return m_running && !m_states.empty(); }
    void Quit() { m_running = false; }

private:
    std::vector<std::unique_ptr<GameState>> m_states;
    bool m_running;
};
