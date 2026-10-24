#pragma once

#include "GameState.h"
#include "PhysicsEngine.h"
#include "Player.h"
#include "Platform.h"
#include "Enemy.h"
#include "Collectable.h"
#include "Vine.h"
#include "AnchorNode.h"
#include "BreakableBarrier.h"
#include "Hazard.h"
#include "RotWave.h"
#include <vector>
#include <memory>

class PlayState : public GameState {
public:
    PlayState();
    ~PlayState() override = default;

    void Init(GameStateManager* manager) override;
    void Cleanup() override;

    void Update(float dt) override;
    void Draw() override;

private:
    PhysicsEngine m_physics;
    std::vector<std::unique_ptr<Entity>> m_entities;

    // Non-owning pointers for fast access
    Player* m_player = nullptr;
    std::vector<Platform*> m_platforms;
    std::vector<Vine*> m_vines;
    std::vector<AnchorNode*> m_anchorNodes;
    std::vector<BreakableBarrier*> m_barriers;
    std::vector<Hazard*> m_hazards;
    std::vector<Collectable*> m_collectables;

    RotWave m_rotWave;
    Camera2D m_camera{};
    Vector2 m_spawnPos{};
    bool m_debugMode;
    int m_score;
    float m_runTimer;
    float m_screenShake;
    bool m_reachedSanctuary;
    Vector2 m_goalSanctuary{};

    void BuildCanopyCourse();
    void DrawBackground() const;
    void DrawParallaxLayer(const Texture2D& tex, float parallaxRate, float yFactor = 0.15f) const;
    void DrawHUD() const;
    void CheckBarrierSmash();
    void CheckHazards();
    void AddScreenShake(float amount);
};
