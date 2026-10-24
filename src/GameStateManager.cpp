#include "GameStateManager.h"

GameStateManager::GameStateManager() : m_running(true) {}

GameStateManager::~GameStateManager() {
    while (!m_states.empty()) {
        m_states.back()->Cleanup();
        m_states.pop_back();
    }
}

void GameStateManager::ChangeState(std::unique_ptr<GameState> state) {
    if (!m_states.empty()) {
        m_states.back()->Cleanup();
        m_states.pop_back();
    }
    if (state) {
        state->Init(this);
        m_states.push_back(std::move(state));
    }
}

void GameStateManager::PushState(std::unique_ptr<GameState> state) {
    if (!m_states.empty()) {
        m_states.back()->Pause();
    }
    if (state) {
        state->Init(this);
        m_states.push_back(std::move(state));
    }
}

void GameStateManager::PopState() {
    if (!m_states.empty()) {
        m_states.back()->Cleanup();
        m_states.pop_back();
    }
    if (!m_states.empty()) {
        m_states.back()->Resume();
    }
}

void GameStateManager::Update(float dt) {
    if (!m_states.empty()) {
        m_states.back()->Update(dt);
    }
}

void GameStateManager::Draw() {
    if (!m_states.empty()) {
        m_states.back()->Draw();
    }
}
