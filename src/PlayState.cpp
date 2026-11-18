#include "PlayState.h"
#include "GameStateManager.h"
#include "MenuState.h"
#include "ResourceManager.h"
#include "ParticleSystem.h"
#include <algorithm>
#include <string>
#include <cmath>

PlayState::PlayState(int levelIndex)
    : m_physics(1100.0f, 950.0f),
      m_levelIndex(levelIndex),
      m_rotWave(520.0f, 10.0f),
      m_debugMode(false),
      m_score(0),
      m_runTimer(0.0f),
      m_screenShake(0.0f),
      m_reachedSanctuary(false) {
    const char* levelNames[] = {
        "High Roost Run",
        "Bramble Sprint",
        "Sanctum Gauntlet",
        "Misdirection Mire",
        "Panic Pendulum",
        "Sunk Cost Spire",
        "Phantom Branch"
    };
    if (m_levelIndex >= 0 && m_levelIndex < static_cast<int>(sizeof(levelNames) / sizeof(levelNames[0]))) {
        m_levelName = levelNames[m_levelIndex];
    } else {
        m_levelIndex = 0;
        m_levelName = levelNames[0];
    }
}

void PlayState::Init(GameStateManager* manager) {
    m_manager = manager;

    // Initialize Camera
    m_camera.target = { 100.0f, 250.0f };
    m_camera.offset = { 380.0f, 320.0f };
    m_camera.rotation = 0.0f;
    m_camera.zoom = 1.0f;

    BuildCanopyCourse();
}

void PlayState::Cleanup() {
    m_entities.clear();
    m_platforms.clear();
    m_vines.clear();
    m_anchorNodes.clear();
    m_barriers.clear();
    m_hazards.clear();
    m_collectables.clear();
    m_player = nullptr;
    m_requiredSnapAnchor = nullptr;
    ParticleSystem::GetInstance().Clear();
}

void PlayState::AddScreenShake(float amount) {
    m_screenShake = std::max(m_screenShake, amount);
}

void PlayState::BuildCanopyCourse() {
    Cleanup();

    m_spawnPos = { 80.0f, 260.0f };
    m_runTimer = 0.0f;
    m_score = 0;
    m_reachedSanctuary = false;
    m_requiredSnapCompleted = false;
    m_rotWave.Reset(520.0f);

    // 1. Create Macaque Player
    auto player = std::make_unique<Player>(m_spawnPos);
    m_player = player.get();
    m_entities.push_back(std::move(player));

    // Entity registration helpers
    auto addPlatform = [this](std::unique_ptr<Platform> plat) {
        m_platforms.push_back(plat.get());
        m_entities.push_back(std::move(plat));
    };

    auto addVine = [this](std::unique_ptr<Vine> vine) {
        m_vines.push_back(vine.get());
        m_entities.push_back(std::move(vine));
    };

    auto addAnchor = [this](std::unique_ptr<AnchorNode> node) {
        m_anchorNodes.push_back(node.get());
        m_entities.push_back(std::move(node));
    };

    auto addBarrier = [this](std::unique_ptr<BreakableBarrier> b) {
        m_barriers.push_back(b.get());
        m_entities.push_back(std::move(b));
    };

    auto addHazard = [this](std::unique_ptr<Hazard> h) {
        m_hazards.push_back(h.get());
        m_entities.push_back(std::move(h));
    };

    auto addCollectable = [this](std::unique_ptr<Collectable> c) {
        m_collectables.push_back(c.get());
        m_entities.push_back(std::move(c));
    };

    addPlatform(std::make_unique<Platform>(
        Vector2{ -400.0f, 490.0f }, Vector2{ 6000.0f, 700.0f }, PlatformType::SOLID));

    if (m_levelIndex == 5) {
        m_spawnPos = { 80.0f, 260.0f };
        m_goalSanctuary = { 1050.0f, -920.0f };
        addPlatform(std::make_unique<Platform>(Vector2{ 20.0f, 320.0f }, Vector2{ 180.0f, 30.0f }, PlatformType::SOLID));
        addVine(std::make_unique<Vine>(Vector2{ 260.0f, 210.0f }, 150.0f));
        addPlatform(std::make_unique<Platform>(Vector2{ 390.0f, 110.0f }, Vector2{ 180.0f, 24.0f }, PlatformType::ONE_WAY));
        addAnchor(std::make_unique<AnchorNode>(Vector2{ 560.0f, 40.0f }, AnchorType::GLOWING_MUSHROOM, 150.0f));
        addVine(std::make_unique<Vine>(Vector2{ 700.0f, -40.0f }, 150.0f));
        addPlatform(std::make_unique<Platform>(Vector2{ 820.0f, -160.0f }, Vector2{ 180.0f, 24.0f }, PlatformType::ONE_WAY));
        addAnchor(std::make_unique<AnchorNode>(Vector2{ 980.0f, -240.0f }, AnchorType::STURDY_BRANCH_KNOT, 150.0f));
        addVine(std::make_unique<Vine>(Vector2{ 800.0f, -350.0f }, 150.0f));
        addPlatform(std::make_unique<Platform>(Vector2{ 570.0f, -470.0f }, Vector2{ 180.0f, 24.0f }, PlatformType::ONE_WAY));
        addAnchor(std::make_unique<AnchorNode>(Vector2{ 400.0f, -550.0f }, AnchorType::GLOWING_MUSHROOM, 150.0f));
        addVine(std::make_unique<Vine>(Vector2{ 260.0f, -650.0f }, 150.0f));
        addPlatform(std::make_unique<Platform>(Vector2{ 390.0f, -770.0f }, Vector2{ 180.0f, 24.0f }, PlatformType::ONE_WAY));
        addVine(std::make_unique<Vine>(Vector2{ 760.0f, -850.0f }, 130.0f));
        addPlatform(std::make_unique<Platform>(Vector2{ 980.0f, -960.0f }, Vector2{ 300.0f, 40.0f }, PlatformType::SOLID));
        auto firstDebris = std::make_unique<Hazard>(Vector2{ 390.0f, -400.0f }, Vector2{ 70.0f, 28.0f });
        firstDebris->SetDropTrigger(-220.0f, 80.0f, 420.0f, 240.0f);
        addHazard(std::move(firstDebris));
        auto secondDebris = std::make_unique<Hazard>(Vector2{ 820.0f, -850.0f }, Vector2{ 70.0f, 28.0f });
        secondDebris->SetDropTrigger(-640.0f, -300.0f, 460.0f, 250.0f);
        addHazard(std::move(secondDebris));
        auto finalSnap = std::make_unique<AnchorNode>(Vector2{ 620.0f, -930.0f }, AnchorType::GLOWING_MUSHROOM, 150.0f);
        m_requiredSnapAnchor = finalSnap.get();
        addAnchor(std::move(finalSnap));
        return;
    }

    float goalX = 3250.0f;
    if (m_levelIndex == 1) {
        goalX = 2800.0f;
    } else if (m_levelIndex == 2) {
        goalX = 3500.0f;
        addHazard(std::make_unique<Hazard>(Vector2{ 1150.0f, 390.0f }, Vector2{ 210.0f, 40.0f }, HazardType::THORN_CLUSTER));
        addHazard(std::make_unique<Hazard>(Vector2{ 2000.0f, 20.0f }, Vector2{ 220.0f, 40.0f }, HazardType::THORN_CLUSTER));
        addBarrier(std::make_unique<BreakableBarrier>(Vector2{ 2280.0f, 90.0f }, Vector2{ 30.0f, 170.0f }, BarrierType::PREDATORY_FLORA));
    }

    // Shared canopy route with level-specific pace changes
    addPlatform(std::make_unique<Platform>(Vector2{ 20.0f, 320.0f }, Vector2{ 180.0f, 30.0f }, PlatformType::SOLID));
    addPlatform(std::make_unique<Platform>(Vector2{ 0.0f, 60.0f }, Vector2{ 500.0f, 35.0f }, PlatformType::SOLID));
    addVine(std::make_unique<Vine>(Vector2{ 340.0f, 75.0f }, 155.0f));
    addPlatform(std::make_unique<Platform>(Vector2{ 480.0f, 50.0f }, Vector2{ 400.0f, 30.0f }, PlatformType::SOLID));
    addVine(std::make_unique<Vine>(Vector2{ 620.0f, 65.0f }, 160.0f));
    addCollectable(std::make_unique<Collectable>(Vector2{ 420.0f, 170.0f }, 100));
    addCollectable(std::make_unique<Collectable>(Vector2{ 730.0f, 130.0f }, 150));
    addHazard(std::make_unique<Hazard>(Vector2{ 280.0f, 460.0f }, Vector2{ 380.0f, 30.0f }, HazardType::THORN_CLUSTER));
    addHazard(std::make_unique<Hazard>(Vector2{ 830.0f, 80.0f }, Vector2{ 60.0f, 180.0f }, HazardType::THORN_CLUSTER));
    addAnchor(std::make_unique<AnchorNode>(Vector2{ 960.0f, 210.0f }, AnchorType::GLOWING_MUSHROOM, 160.0f));
    addCollectable(std::make_unique<Collectable>(Vector2{ 960.0f, 100.0f }, 200));
    addPlatform(std::make_unique<Platform>(Vector2{ 1160.0f, -40.0f }, Vector2{ 70.0f, 440.0f }, PlatformType::TREE_BARK));
    addVine(std::make_unique<Vine>(Vector2{ 1380.0f, 60.0f }, 165.0f));
    addCollectable(std::make_unique<Collectable>(Vector2{ 1300.0f, 180.0f }, 150));
    addPlatform(std::make_unique<Platform>(Vector2{ 1520.0f, 120.0f }, Vector2{ 280.0f, 25.0f }, PlatformType::SOLID));
    addPlatform(std::make_unique<Platform>(Vector2{ 1520.0f, 280.0f }, Vector2{ 280.0f, 25.0f }, PlatformType::SOLID));
    addBarrier(std::make_unique<BreakableBarrier>(Vector2{ 1620.0f, 145.0f }, Vector2{ 24.0f, 135.0f }, BarrierType::BRITTLE_WOOD));
    addBarrier(std::make_unique<BreakableBarrier>(Vector2{ 1740.0f, 145.0f }, Vector2{ 24.0f, 135.0f }, BarrierType::BRITTLE_WOOD));
    addCollectable(std::make_unique<Collectable>(Vector2{ 1680.0f, 200.0f }, 250));
    addHazard(std::make_unique<Hazard>(Vector2{ 1860.0f, 400.0f }, Vector2{ 500.0f, 40.0f }, HazardType::THORN_CLUSTER));
    addHazard(std::make_unique<Hazard>(Vector2{ 1980.0f, 20.0f }, Vector2{ 350.0f, 40.0f }, HazardType::THORN_CLUSTER));
    addAnchor(std::make_unique<AnchorNode>(Vector2{ 1980.0f, 260.0f }, AnchorType::GLOWING_MUSHROOM, 150.0f));
    addAnchor(std::make_unique<AnchorNode>(Vector2{ 2220.0f, 140.0f }, AnchorType::STURDY_BRANCH_KNOT, 150.0f));
    addCollectable(std::make_unique<Collectable>(Vector2{ 2100.0f, 180.0f }, 250));
    addBarrier(std::make_unique<BreakableBarrier>(Vector2{ 2440.0f, 100.0f }, Vector2{ 30.0f, 150.0f }, BarrierType::PREDATORY_FLORA));
    addVine(std::make_unique<Vine>(Vector2{ 2680.0f, 40.0f }, 175.0f));
    addAnchor(std::make_unique<AnchorNode>(Vector2{ 2940.0f, 130.0f }, AnchorType::GLOWING_MUSHROOM, 160.0f));

    if (m_levelIndex == 1) {
        addHazard(std::make_unique<Hazard>(Vector2{ 2500.0f, 420.0f }, Vector2{ 220.0f, 26.0f }, HazardType::THORN_CLUSTER));
        addCollectable(std::make_unique<Collectable>(Vector2{ 2580.0f, 80.0f }, 220));
    } else if (m_levelIndex == 2) {
        addHazard(std::make_unique<Hazard>(Vector2{ 2550.0f, 310.0f }, Vector2{ 260.0f, 30.0f }, HazardType::THORN_CLUSTER));
        addHazard(std::make_unique<Hazard>(Vector2{ 3040.0f, 100.0f }, Vector2{ 110.0f, 160.0f }, HazardType::THORN_CLUSTER));
        addCollectable(std::make_unique<Collectable>(Vector2{ 3180.0f, 100.0f }, 350));
    } else if (m_levelIndex == 3) {
        addAnchor(std::make_unique<AnchorNode>(Vector2{ 500.0f, 210.0f }, AnchorType::GLOWING_MUSHROOM, 160.0f, true));
        addAnchor(std::make_unique<AnchorNode>(Vector2{ 1280.0f, 190.0f }, AnchorType::GLOWING_MUSHROOM, 155.0f, true));
        addAnchor(std::make_unique<AnchorNode>(Vector2{ 2160.0f, 170.0f }, AnchorType::STURDY_BRANCH_KNOT, 155.0f, true));
        addHazard(std::make_unique<Hazard>(Vector2{ 2700.0f, 430.0f }, Vector2{ 280.0f, 26.0f }, HazardType::THORN_CLUSTER));
    } else if (m_levelIndex == 4) {
        const Vector2 logStarts[] = { { 700.0f, 230.0f }, { 1350.0f, 260.0f }, { 2100.0f, 220.0f } };
        const Vector2 logEnds[] = { { 900.0f, 230.0f }, { 1650.0f, 260.0f }, { 2440.0f, 220.0f } };
        const Vector2 logSizes[] = { { 110.0f, 36.0f }, { 120.0f, 36.0f }, { 130.0f, 38.0f } };
        const float logSpeeds[] = { 210.0f, 240.0f, 260.0f };
        for (int i = 0; i < 3; ++i) {
            addHazard(std::make_unique<Hazard>(logStarts[i], logEnds[i], logSizes[i], logSpeeds[i]));
            auto pivot = std::make_unique<AnchorNode>(
                Vector2{ logStarts[i].x + logSizes[i].x * 0.5f, logStarts[i].y + logSizes[i].y * 0.5f },
                AnchorType::GLOWING_MUSHROOM, 150.0f);
            pivot->SetMovingPath({ logEnds[i].x + logSizes[i].x * 0.5f, logEnds[i].y + logSizes[i].y * 0.5f }, logSpeeds[i]);
            addAnchor(std::move(pivot));
        }
    } else if (m_levelIndex == 6) {
        auto addPhantomPlatform = [&addPlatform](Vector2 position, Vector2 size) {
            auto platform = std::make_unique<Platform>(position, size, PlatformType::ONE_WAY);
            platform->SetRevealOnApproach();
            addPlatform(std::move(platform));
        };
        addPhantomPlatform({ 2780.0f, 290.0f }, { 130.0f, 20.0f });
        addPhantomPlatform({ 2960.0f, 220.0f }, { 130.0f, 20.0f });
        addPhantomPlatform({ 3100.0f, 280.0f }, { 130.0f, 20.0f });
        addHazard(std::make_unique<Hazard>(Vector2{ 2920.0f, 360.0f }, Vector2{ 320.0f, 24.0f }, HazardType::THORN_CLUSTER));
        addCollectable(std::make_unique<Collectable>(Vector2{ 3010.0f, 150.0f }, 350));
    }

    m_goalSanctuary = { goalX, 160.0f };
    addPlatform(std::make_unique<Platform>(Vector2{ goalX - 70.0f, 210.0f }, Vector2{ 360.0f, 60.0f }, PlatformType::SOLID));
    addPlatform(std::make_unique<Platform>(Vector2{ goalX + 20.0f, 120.0f }, Vector2{ 160.0f, 20.0f }, PlatformType::ONE_WAY));
    addCollectable(std::make_unique<Collectable>(Vector2{ goalX + 180.0f, 70.0f }, 500));
}

void PlayState::CheckBarrierSmash() {
    if (!m_player) return;

    Rectangle playerBounds = m_player->GetBounds();
    bool canSmash = m_player->IsPouncing() || (m_player->GetSpeed() > 650.0f);

    for (auto* b : m_barriers) {
        if (!b || !b->IsActive() || b->IsBroken()) continue;

        if (PhysicsEngine::CheckAABB(playerBounds, b->GetBounds())) {
            if (canSmash) {
                b->Break();
                AddScreenShake(9.0f);
                m_score += 150;
            } else {
                // Not fast enough: bounce off or block
                Vector2 vel = m_player->GetVelocity();
                vel.x = -vel.x * 0.4f;
                m_player->SetVelocity(vel);
            }
        }
    }
}

void PlayState::CheckHazards() {
    if (!m_player || !m_player->IsActive()) return;

    Rectangle playerBounds = m_player->GetBounds();

    // 1. Thorn Hazards
    for (auto* h : m_hazards) {
        if (!h || !h->IsActive()) continue;
        if (m_levelIndex == 4 && h->IsMoving() &&
            m_player->GetMovementState() == PlayerMovementState::TAIL_PIVOTING) {
            continue;
        }

        if (PhysicsEngine::CheckAABB(playerBounds, h->GetBounds())) {
            // Hit thorns: burst & reset
            Vector2 pCenter = { playerBounds.x + playerBounds.width * 0.5f, playerBounds.y + playerBounds.height * 0.5f };
            ParticleSystem::GetInstance().EmitBurst(pCenter, 25, Color{ 220, 20, 60, 255 }, 120.0f, 320.0f, 0.4f);
            AddScreenShake(12.0f);
            m_player->Reset(m_spawnPos);
            m_rotWave.Reset(520.0f);
            m_requiredSnapCompleted = false;
            return;
        }
    }

    // 2. Rising Bio-Engineered Rot Wave (Spore sea / left creeping rot)
    if (m_rotWave.IsSubmerged(m_player->GetPosition(), m_player->GetSize())) {
        Vector2 pCenter = { playerBounds.x + playerBounds.width * 0.5f, playerBounds.y + playerBounds.height * 0.5f };
        ParticleSystem::GetInstance().EmitBurst(pCenter, 30, Color{ 142, 68, 173, 255 }, 140.0f, 350.0f, 0.45f);
        AddScreenShake(14.0f);
        m_player->Reset(m_spawnPos);
        m_rotWave.Reset(520.0f);
        m_requiredSnapCompleted = false;
        return;
    }

    // 3. Fall out of world boundary
    if (m_player->GetPosition().y > 800.0f) {
        m_player->Reset(m_spawnPos);
        m_rotWave.Reset(520.0f);
        m_requiredSnapCompleted = false;
    }
}

void PlayState::Update(float dt) {
    if (!m_reachedSanctuary) {
        m_runTimer += dt;
    }

    // Toggle Debug Overlay
    if (IsKeyPressed(KEY_F1) || IsKeyPressed(KEY_TAB)) {
        m_debugMode = !m_debugMode;
    }

    // Quick Restart / Return to menu
    if (IsKeyPressed(KEY_R)) {
        if (m_player) {
            m_player->Reset(m_spawnPos);
            m_rotWave.Reset(520.0f);
            m_reachedSanctuary = false;
            m_requiredSnapCompleted = false;
        }
    }
    Rectangle menuButton = {
        static_cast<float>(GetScreenWidth() - 150),
        14.0f,
        136.0f,
        36.0f
    };
    bool clickedMenuButton = IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
                             CheckCollisionPointRec(GetMousePosition(), menuButton);
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_M) || clickedMenuButton) {
        if (m_manager) {
            m_manager->ChangeState(std::make_unique<MenuState>());
            return;
        }
    }

    // 1. Update Vines (pendulum physics)
    for (auto* vine : m_vines) {
        if (vine && vine->IsActive()) {
            vine->Update(dt);
        }
    }

    if (m_levelIndex == 6 && m_player) {
        float approachX = m_player->GetPosition().x + 260.0f;
        for (auto* platform : m_platforms) {
            if (platform && platform->CanBeRevealed() && approachX >= platform->GetPosition().x) {
                platform->RevealFor(2.0f);
            }
        }
    }
    for (auto* platform : m_platforms) {
        if (platform && platform->IsActive()) {
            platform->Update(dt);
        }
    }

    // 2. Update Anchor Nodes
    for (auto* node : m_anchorNodes) {
        if (node && node->IsActive()) {
            node->Update(dt);
        }
    }

    // 3. Update Breakable Barriers
    for (auto* b : m_barriers) {
        if (b && b->IsActive()) {
            b->Update(dt);
        }
    }

    // 4. Update Hazards
    for (auto* h : m_hazards) {
        if (h && h->IsActive()) {
            if (m_levelIndex == 5 && m_player) {
                h->TryTriggerDrop(m_player->GetPosition(), m_player->GetVelocity());
            }
            h->Update(dt);
        }
    }

    // 5. Update Player with Brachiation & Tail Snap
    if (m_player && m_player->IsActive()) {
        m_player->HandleInput(dt, m_vines, m_anchorNodes);
        if (m_requiredSnapAnchor && !m_requiredSnapAnchor->CanSnap()) {
            m_requiredSnapCompleted = true;
        }

        // When free falling / jumping / sliding, physics engine handles kinematic collision
        PlayerMovementState state = m_player->GetMovementState();
        if (m_levelIndex == 3 && state == PlayerMovementState::WALL_SLIDING &&
            m_player->GetPosition().x >= 1120.0f && m_player->GetPosition().x <= 1160.0f) {
            m_player->SetVelocityY(500.0f);
        }
        if (state == PlayerMovementState::AIRBORNE || state == PlayerMovementState::WALL_SLIDING) {
            m_physics.ApplyGravity(m_player, dt);
            m_physics.UpdateEntityPhysics(m_player, m_platforms, dt);
        } else if (state == PlayerMovementState::POUNCING) {
            // High speed pounce movement
            Vector2 p = m_player->GetPosition();
            Vector2 v = m_player->GetVelocity();
            m_player->SetPosition({ p.x + v.x * dt, p.y + v.y * dt });
        }

        m_player->Update(dt);
    }

    // 6. Check Barrier Smashes & Lethal Hazards
    CheckBarrierSmash();
    CheckHazards();

    // 7. Update Rising Rot Wave
    if (m_player) {
        m_rotWave.Update(dt, m_player->GetPosition());
    }

    // 8. Update Collectables & Proximity Pickup
    for (auto* item : m_collectables) {
        if (item && item->IsActive()) {
            item->Update(dt);
            if (m_player && PhysicsEngine::CheckAABB(m_player->GetBounds(), item->GetBounds())) {
                item->Collect();
                m_score += item->GetScoreValue();
                Vector2 c = { item->GetBounds().x + 8.0f, item->GetBounds().y + 8.0f };
                ParticleSystem::GetInstance().EmitBurst(c, 12, GOLD, 80.0f, 220.0f, 0.3f);
            }
        }
    }

    // 9. Check Sanctuary Goal Reached
    if (m_player && !m_reachedSanctuary) {
        if (m_player->GetPosition().x >= m_goalSanctuary.x && m_player->GetPosition().y <= m_goalSanctuary.y + 60.0f &&
            (m_levelIndex != 5 || m_requiredSnapCompleted)) {
            m_reachedSanctuary = true;
            ParticleSystem::GetInstance().EmitBurst(m_goalSanctuary, 60, GOLD, 150.0f, 450.0f, 0.8f);
            AddScreenShake(10.0f);
        }
    }

    // 10. Update Particle Engine
    ParticleSystem::GetInstance().Update(dt);

    // 11. Camera Tracking with Lookahead & Screen Shake
    if (m_player) {
        Vector2 pVel = m_player->GetVelocity();
        // Dynamic lookahead: camera anticipates high velocity travel
        float lookAheadX = std::clamp(pVel.x * 0.35f, -220.0f, 280.0f);
        float lookAheadY = std::clamp(pVel.y * 0.20f, -120.0f, 120.0f);

        Vector2 target = {
            m_player->GetPosition().x + m_player->GetSize().x * 0.5f + lookAheadX,
            m_player->GetPosition().y + m_player->GetSize().y * 0.5f + lookAheadY - 30.0f
        };

        // Smooth camera lerp
        m_camera.target.x += (target.x - m_camera.target.x) * 7.0f * dt;
        m_camera.target.y += (target.y - m_camera.target.y) * 5.0f * dt;

        // Apply screen shake
        if (m_screenShake > 0.0f) {
            float sx = ((rand() % 100) / 100.0f - 0.5f) * m_screenShake * 2.0f;
            float sy = ((rand() % 100) / 100.0f - 0.5f) * m_screenShake * 2.0f;
            m_camera.target.x += sx;
            m_camera.target.y += sy;
            m_screenShake = std::max(0.0f, m_screenShake - dt * 25.0f);
        }
    }
}

void PlayState::DrawBackground() const {
    Texture2D& bg0 = ResourceManager::GetInstance().GetTexture("bg_layer_0");
    Texture2D& bg1 = ResourceManager::GetInstance().GetTexture("bg_layer_1");
    Texture2D& bg2 = ResourceManager::GetInstance().GetTexture("bg_layer_2");

    // Atmospheric ancient canopy night fill
    ClearBackground(Color{ 10, 14, 22, 255 });

    if (bg0.id != 0) DrawParallaxLayer(bg0, 0.06f, 0.03f);
    if (bg1.id != 0) DrawParallaxLayer(bg1, 0.22f, 0.09f);
    if (bg2.id != 0) DrawParallaxLayer(bg2, 0.48f, 0.16f);
}

void PlayState::DrawParallaxLayer(const Texture2D& tex, float parallaxRate, float yFactor) const {
    if (tex.id == 0) return;

    float scale = static_cast<float>(GetScreenHeight()) / static_cast<float>(tex.height);
    float drawWidth = tex.width * scale;
    float drawHeight = tex.height * scale;

    float camX = m_camera.target.x;
    float offset = -std::fmod(camX * parallaxRate, drawWidth);
    if (offset > 0.0f) offset -= drawWidth;

    float camY = m_camera.target.y;
    float yOffset = std::clamp((250.0f - camY) * yFactor, -60.0f, 60.0f);

    for (float x = offset - drawWidth; x < GetScreenWidth() + drawWidth; x += drawWidth) {
        Rectangle src = { 0.0f, 0.0f, static_cast<float>(tex.width), static_cast<float>(tex.height) };
        Rectangle dest = { x, yOffset, drawWidth, drawHeight };
        DrawTexturePro(tex, src, dest, { 0.0f, 0.0f }, 0.0f, WHITE);
    }
}

void PlayState::DrawJungleScenery() const {
    constexpr float groundY = 490.0f;
    constexpr float backTreeSpacing = 590.0f;
    constexpr float frontTreeSpacing = 430.0f;
    const float viewLeft = m_camera.target.x - m_camera.offset.x - 160.0f;
    const float viewRight = viewLeft + GetScreenWidth() + 320.0f;
    const int firstBackTree = static_cast<int>(std::floor(viewLeft / backTreeSpacing));
    const int lastBackTree = static_cast<int>(std::ceil(viewRight / backTreeSpacing));

    auto drawTree = [](int treeIndex, float centerX, float baseY, float height, Color tint) {
        const int spriteIndex = ((treeIndex % 8) + 8) % 8;
        Texture2D& tree = ResourceManager::GetInstance().GetTexture(
            "jungle_tree_" + std::to_string(spriteIndex));
        if (tree.id == 0 || tree.height <= 0) return;

        const float width = height * static_cast<float>(tree.width) / static_cast<float>(tree.height);
        Rectangle source = { 0.0f, 0.0f, static_cast<float>(tree.width), static_cast<float>(tree.height) };
        Rectangle destination = { centerX - width * 0.5f, baseY - height, width, height };
        DrawTexturePro(tree, source, destination, { 0.0f, 0.0f }, 0.0f, tint);
    };

    // Distant tree silhouettes add depth while keeping the existing parallax untouched.
    for (int tree = firstBackTree; tree <= lastBackTree; ++tree) {
        const float x = tree * backTreeSpacing + 240.0f;
        drawTree(tree + 3, x, groundY + 18.0f, 410.0f, Color{ 125, 155, 125, 190 });
    }

    const int firstFrontTree = static_cast<int>(std::floor(viewLeft / frontTreeSpacing));
    const int lastFrontTree = static_cast<int>(std::ceil(viewRight / frontTreeSpacing));
    for (int tree = firstFrontTree; tree <= lastFrontTree; ++tree) {
        const float x = tree * frontTreeSpacing + 205.0f;
        drawTree(tree, x, groundY + 25.0f, 500.0f, WHITE);
    }

    const float screenLeft = m_camera.target.x - m_camera.offset.x - 100.0f;
    const float screenRight = screenLeft + GetScreenWidth() + 200.0f;
    const float canopyY = m_camera.target.y - m_camera.offset.y + 8.0f;
    const int firstCanopy = static_cast<int>(std::floor(screenLeft / 55.0f));
    const int lastCanopy = static_cast<int>(std::ceil(screenRight / 55.0f));
    for (int leaf = firstCanopy; leaf <= lastCanopy; ++leaf) {
        const float x = leaf * 55.0f;
        const float radiusX = 44.0f + static_cast<float>((leaf * 17) & 23);
        const float radiusY = 38.0f + static_cast<float>((leaf * 11) & 19);
        DrawEllipse(static_cast<int>(x), static_cast<int>(canopyY + 13.0f),
                    radiusX, radiusY, Color{ 12, 57, 34, 250 });
        DrawEllipse(static_cast<int>(x - 19.0f), static_cast<int>(canopyY + 4.0f),
                    radiusX * 0.64f, radiusY * 0.72f, Color{ 24, 91, 43, 255 });
        DrawEllipse(static_cast<int>(x + 17.0f), static_cast<int>(canopyY + 1.0f),
                    radiusX * 0.53f, radiusY * 0.63f, Color{ 39, 111, 49, 245 });
    }

    for (int vine = firstCanopy; vine <= lastCanopy; vine += 3) {
        const float x = vine * 55.0f + 20.0f;
        const float length = 50.0f + static_cast<float>((vine * 29) & 95);
        Vector2 previous = { x, canopyY + 17.0f };
        for (int segment = 1; segment <= 5; ++segment) {
            Vector2 next = {
                x + std::sin(segment * 0.72f + vine) * 7.0f,
                canopyY + 17.0f + length * segment / 5.0f
            };
            DrawLineEx(previous, next, 3.0f, Color{ 43, 111, 49, 245 });
            if (segment == 2 || segment == 4) {
                DrawEllipse(static_cast<int>(next.x + 6.0f), static_cast<int>(next.y),
                            8.0f, 4.0f, Color{ 81, 151, 59, 255 });
            }
            previous = next;
        }
    }

    const int firstClump = static_cast<int>(std::floor((m_camera.target.x - 1000.0f) / 170.0f));
    const int lastClump = static_cast<int>(std::ceil((m_camera.target.x + 1000.0f) / 170.0f));
    for (int clump = firstClump; clump <= lastClump; ++clump) {
        const float x = clump * 170.0f;
        DrawEllipse(static_cast<int>(x), static_cast<int>(groundY), 38.0f, 25.0f,
                    Color{ 25, 82, 43, 220 });
        DrawEllipse(static_cast<int>(x + 25.0f), static_cast<int>(groundY + 7.0f), 32.0f, 21.0f,
                    Color{ 35, 112, 50, 220 });
        DrawEllipse(static_cast<int>(x - 23.0f), static_cast<int>(groundY + 10.0f), 28.0f, 19.0f,
                    Color{ 47, 126, 54, 220 });
    }
}

void PlayState::Draw() {
    // 1. Draw Canopy Parallax Backdrops
    DrawBackground();

    // 2. Draw World Scene
    BeginMode2D(m_camera);

    // Jungle vegetation is part of the level scene; the existing parallax remains unchanged.
    DrawJungleScenery();

    // Platforms & Ancient Redwood Trunks
    for (auto* plat : m_platforms) {
        if (plat && plat->IsActive()) {
            plat->Draw();
        }
    }

    // Hazards (Thorns)
    for (auto* h : m_hazards) {
        if (h && h->IsActive()) {
            h->Draw();
        }
    }

    // Breakable barriers
    for (auto* b : m_barriers) {
        if (b && b->IsActive()) {
            b->Draw();
        }
    }

    // Anchor Nodes
    for (auto* node : m_anchorNodes) {
        if (node && node->IsActive()) {
            node->Draw();
        }
    }

    // Vines
    for (auto* vine : m_vines) {
        if (vine && vine->IsActive()) {
            vine->Draw();
        }
    }

    // Collectibles
    for (auto* item : m_collectables) {
        if (item && item->IsActive()) {
            item->Draw();
        }
    }

    // Rising Bio-Engineered Rot Spore Sea
    m_rotWave.Draw(m_camera);

    // Sanctuary Goal Haven (Sacred golden canopy shrine)
    DrawCircleGradient(static_cast<int>(m_goalSanctuary.x + 80.0f), static_cast<int>(m_goalSanctuary.y + 20.0f),
                       90.0f, Color{ 255, 215, 0, 80 }, BLANK);
    DrawRectangle(static_cast<int>(m_goalSanctuary.x + 70.0f), static_cast<int>(m_goalSanctuary.y - 40.0f), 8, 55, Color{ 255, 215, 0, 255 });
    DrawTriangle({ m_goalSanctuary.x + 78.0f, m_goalSanctuary.y - 40.0f },
                 { m_goalSanctuary.x + 115.0f, m_goalSanctuary.y - 25.0f },
                 { m_goalSanctuary.x + 78.0f, m_goalSanctuary.y - 10.0f }, Color{ 46, 204, 113, 255 });
    DrawText("CANOPY SANCTUARY", static_cast<int>(m_goalSanctuary.x + 10.0f), static_cast<int>(m_goalSanctuary.y - 60.0f), 12, GOLD);

    // Particles (Spore embers, speed trails, wood splinters)
    ParticleSystem::GetInstance().Draw();

    // Player Macaque
    if (m_player && m_player->IsActive()) {
        m_player->Draw();
    }

    // Debug View
    if (m_debugMode) {
        if (m_player) m_player->DrawDebug();
        for (auto* v : m_vines) if (v) v->DrawDebug();
        for (auto* a : m_anchorNodes) if (a) a->DrawDebug();
        for (auto* b : m_barriers) if (b) b->DrawDebug();
        for (auto* h : m_hazards) if (h) h->DrawDebug();
    }

    EndMode2D();

    if (m_levelIndex == 6) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{ 170, 185, 205, 30 });
        DrawCircleGradient(GetScreenWidth() / 4, GetScreenHeight() / 2, 190.0f,
                           Color{ 185, 200, 220, 32 }, BLANK);
        DrawCircleGradient(GetScreenWidth() * 3 / 4, GetScreenHeight() / 3, 240.0f,
                           Color{ 185, 200, 220, 24 }, BLANK);
    }

    // 3. Draw Canopy Snap Kinetic HUD
    DrawHUD();
}

void PlayState::DrawHUD() const {
    if (!m_player) return;

    // --- TOP-LEFT STATUS & GAUGES ---
    DrawRectangle(14, 14, 380, 124, Color{ 14, 18, 26, 235 });
    DrawRectangleLines(14, 14, 380, 124, Color{ 45, 60, 85, 255 });

    // Title & Logo
    DrawText("CANOPY SNAP", 26, 22, 16, Color{ 0, 255, 220, 255 });
    DrawText(TextFormat("LEVEL: %s", m_levelName.c_str()), 26, 42, 10, Color{ 170, 210, 255, 255 });

    // Kinetic State Pill
    PlayerMovementState state = m_player->GetMovementState();
    const char* stateStr = "AIRBORNE";
    Color stateColor = SKYBLUE;
    if (state == PlayerMovementState::BRACHIATING) {
        stateStr = "BRACHIATING [SWING]";
        stateColor = Color{ 46, 204, 113, 255 };
    } else if (state == PlayerMovementState::TAIL_PIVOTING) {
        stateStr = "TAIL PIVOT [WHIP ARC]";
        stateColor = Color{ 0, 255, 230, 255 };
    } else if (state == PlayerMovementState::WALL_SLIDING) {
        stateStr = "BARK CLING";
        stateColor = Color{ 230, 126, 34, 255 };
    } else if (state == PlayerMovementState::POUNCING) {
        stateStr = "POUNCE SMASH!";
        stateColor = Color{ 255, 69, 0, 255 };
    }

    DrawRectangle(150, 20, 170, 20, Color{ 22, 28, 40, 255 });
    DrawRectangleLines(150, 20, 170, 20, stateColor);
    DrawText(stateStr, 158, 24, 11, stateColor);

    // Speedometer
    float currentSpeed = m_player->GetSpeed();
    DrawText(TextFormat("VELOCITY: %.0f px/s", currentSpeed), 26, 56, 12, (currentSpeed > 450.0f) ? Color{ 255, 215, 0, 255 } : LIGHTGRAY);

    // Speed bar
    float speedRatio = std::clamp(currentSpeed / 900.0f, 0.0f, 1.0f);
    DrawRectangle(150, 58, 170, 8, Color{ 30, 35, 48, 255 });
    DrawRectangle(150, 58, static_cast<int>(170.0f * speedRatio), 8, (currentSpeed > 500.0f) ? Color{ 255, 140, 0, 255 } : Color{ 52, 152, 219, 255 });

    // POUNCE METER GAUGE
    float pounceMeter = m_player->GetPounceMeter();
    DrawText("POUNCE METER:", 26, 78, 12, (pounceMeter >= 25.0f) ? Color{ 255, 165, 0, 255 } : GRAY);

    DrawRectangle(150, 78, 170, 14, Color{ 30, 35, 48, 255 });
    Color pounceColor = (pounceMeter >= 25.0f) ? Color{ 255, 140, 0, 255 } : Color{ 160, 100, 30, 200 };
    DrawRectangle(150, 78, static_cast<int>(170.0f * (pounceMeter / 100.0f)), 14, pounceColor);
    DrawRectangleLines(150, 78, 170, 14, Color{ 70, 80, 105, 255 });

    if (pounceMeter >= 25.0f) {
        DrawText("READY [Q / F / Shift]", 162, 80, 10, WHITE);
    }

    // Run Timer & Score
    DrawText(TextFormat("TIME: %.1fs  |  SCORE: %d", m_runTimer, m_score), 26, 106, 11, GOLD);

    // --- TOP-RIGHT EXIT TO MENU BUTTON ---
    Rectangle menuButton = {
        static_cast<float>(GetScreenWidth() - 150),
        14.0f,
        136.0f,
        36.0f
    };
    bool hovered = CheckCollisionPointRec(GetMousePosition(), menuButton);
    Color menuButtonColor = hovered ? Color{ 0, 130, 120, 255 } : Color{ 18, 38, 48, 245 };
    DrawRectangleRec(menuButton, menuButtonColor);
    DrawRectangleLinesEx(menuButton, 2.0f, Color{ 0, 255, 220, 255 });
    DrawText("MAIN MENU", static_cast<int>(menuButton.x + 23), static_cast<int>(menuButton.y + 11), 12, WHITE);
    if (m_levelIndex == 5 && !m_requiredSnapCompleted) {
        DrawText("OBJECTIVE: FIND THE HIDDEN TAIL-SNAP", GetScreenWidth() - 355, 60, 11, GOLD);
    }

    // --- BOTTOM CONTROLS QUICK BAR ---
    DrawRectangle(14, GetScreenHeight() - 38, 850, 26, Color{ 14, 18, 26, 215 });
    DrawText("[Space/Up/W] Jump  |  [Hold Space/LMB] Swing  |  [E/RMB/Shift] Tail Snap  |  [Q/F] Pounce  |  [R] Reset  |  [Esc/M] Menu",
             22, GetScreenHeight() - 31, 10, Color{ 180, 200, 220, 255 });

    // --- WARNING PROXIMITY TO TOXIC SPORE SEA ---
    float distToRot = m_rotWave.GetCurrentY() - (m_player->GetPosition().y + m_player->GetSize().y);
    if (distToRot < 140.0f && distToRot > 0.0f) {
        float alertPulse = std::sin(GetTime() * 10.0f) * 0.5f + 0.5f;
        Color warnCol = Color{ 231, 76, 60, static_cast<unsigned char>(180 + alertPulse * 75) };
        DrawRectangle(GetScreenWidth() / 2 - 130, 18, 260, 26, Color{ 35, 10, 20, 220 });
        DrawRectangleLines(GetScreenWidth() / 2 - 130, 18, 260, 26, warnCol);
        DrawText("! SPORE ROT TIDE RISING !", GetScreenWidth() / 2 - 95, 24, 12, warnCol);
    }

    // --- VICTORY SANCTUARY SCREEN ---
    if (m_reachedSanctuary) {
        DrawRectangle(GetScreenWidth() / 2 - 240, GetScreenHeight() / 2 - 90, 480, 180, Color{ 12, 18, 28, 245 });
        DrawRectangleLines(GetScreenWidth() / 2 - 240, GetScreenHeight() / 2 - 90, 480, 180, GOLD);

        DrawText("CANOPY SANCTUARY REACHED!", GetScreenWidth() / 2 - 180, GetScreenHeight() / 2 - 65, 20, GOLD);
        DrawText("You escaped the bio-engineered spore rot!", GetScreenWidth() / 2 - 165, GetScreenHeight() / 2 - 35, 13, RAYWHITE);
        DrawText(TextFormat("Run Time: %.2f seconds   Score: %d", m_runTimer, m_score), GetScreenWidth() / 2 - 140, GetScreenHeight() / 2 - 5, 13, Color{ 0, 255, 220, 255 });
        DrawText("Press [R] to race through the canopy again!   [M] for menu", GetScreenWidth() / 2 - 180, GetScreenHeight() / 2 + 35, 12, Color{ 255, 215, 0, 255 });
    }
}
