#pragma once

class GameStateManager;

class GameState {
public:
    virtual ~GameState() = default;

    virtual void Init(GameStateManager* manager) = 0;
    virtual void Cleanup() = 0;

    virtual void Pause() {}
    virtual void Resume() {}

    virtual void Update(float dt) = 0;
    virtual void Draw() = 0;

protected:
    GameStateManager* m_manager = nullptr;
};
