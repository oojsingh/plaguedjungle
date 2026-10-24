#include "PlayState.h"
#include "ResourceManager.h"
#include "ParticleSystem.h"
#include <algorithm>
#include <string>
#include <cmath>

PlayState::PlayState()
    : m_physics(1100.0f, 950.0f),
      m_rotWave(520.0f, 10.0f),
      m_debugMode(false),
      m_score(0),
      m_runTimer(0.0f),
      m_screenShake(0.0f),
      m_reachedSanctuary(false) {}

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

    // =========================================================================
    // CANOPY SNAP: LEVEL 1 - "THE ASCENDING SPORE SURGE"
    // High-momentum canopy course requiring brachiation, tail snaps & pounces
    // =========================================================================

    // --- ZONE 1: Starting High Roost & First Brachiation Vine ---
    // Start Roost branch
    addPlatform(std::make_unique<Platform>(Vector2{ 20.0f, 320.0f }, Vector2{ 180.0f, 30.0f }, PlatformType::SOLID));
    // High overhead canopy branch
    addPlatform(std::make_unique<Platform>(Vector2{ 0.0f, 60.0f }, Vector2{ 500.0f, 35.0f }, PlatformType::SOLID));

    // First Vine (Teaches grab & pump momentum)
    addVine(std::make_unique<Vine>(Vector2{ 340.0f, 75.0f }, 155.0f));

    // --- ZONE 2: Double Vine Slingshot & Launch Sweet-Spot ---
    addPlatform(std::make_unique<Platform>(Vector2{ 480.0f, 50.0f }, Vector2{ 400.0f, 30.0f }, PlatformType::SOLID));
    addVine(std::make_unique<Vine>(Vector2{ 620.0f, 65.0f }, 160.0f));

    // Spore gems along optimal swing trajectory
    addCollectable(std::make_unique<Collectable>(Vector2{ 420.0f, 170.0f }, 100));
    addCollectable(std::make_unique<Collectable>(Vector2{ 730.0f, 130.0f }, 150));

    // Floor hazard spikes / thorn clusters under the vines
    addHazard(std::make_unique<Hazard>(Vector2{ 280.0f, 460.0f }, Vector2{ 380.0f, 30.0f }, HazardType::THORN_CLUSTER));

    // --- ZONE 3: The First Tail Pivot Corner (The Glowing Mushroom Chasm) ---
    // Overhead jagged barrier forces player downwards then sharp pivot upwards
    addHazard(std::make_unique<Hazard>(Vector2{ 830.0f, 80.0f }, Vector2{ 60.0f, 180.0f }, HazardType::THORN_CLUSTER));

    // Glowing Anchor Node (Snaps tail and whips macaque around the thorn obstacle!)
    addAnchor(std::make_unique<AnchorNode>(Vector2{ 960.0f, 210.0f }, AnchorType::GLOWING_MUSHROOM, 160.0f));
    addCollectable(std::make_unique<Collectable>(Vector2{ 960.0f, 100.0f }, 200));

    // --- ZONE 4: Vertical Redwood Bark Trunk & Wall Cling / Kick ---
    // Giant vertical trunk that player clings to and slides down
    addPlatform(std::make_unique<Platform>(Vector2{ 1160.0f, -40.0f }, Vector2{ 70.0f, 440.0f }, PlatformType::TREE_BARK));

    // Vine immediately past the wall kick
    addVine(std::make_unique<Vine>(Vector2{ 1380.0f, 60.0f }, 165.0f));
    addCollectable(std::make_unique<Collectable>(Vector2{ 1300.0f, 180.0f }, 150));

    // --- ZONE 5: The Brittle Barrier Smash (Pounce Speed Test) ---
    // A narrow wooden canopy tunnel blocked by brittle barricades
    addPlatform(std::make_unique<Platform>(Vector2{ 1520.0f, 120.0f }, Vector2{ 280.0f, 25.0f }, PlatformType::SOLID));
    addPlatform(std::make_unique<Platform>(Vector2{ 1520.0f, 280.0f }, Vector2{ 280.0f, 25.0f }, PlatformType::SOLID));

    // Two Brittle Barriers blocking the tunnel! Requires cashing in Pounce Meter to smash through!
    addBarrier(std::make_unique<BreakableBarrier>(Vector2{ 1620.0f, 145.0f }, Vector2{ 24.0f, 135.0f }, BarrierType::BRITTLE_WOOD));
    addBarrier(std::make_unique<BreakableBarrier>(Vector2{ 1740.0f, 145.0f }, Vector2{ 24.0f, 135.0f }, BarrierType::BRITTLE_WOOD));
    addCollectable(std::make_unique<Collectable>(Vector2{ 1680.0f, 200.0f }, 250));

    // --- ZONE 6: The Thorn Maze & Dual Tail-Snap S-Curve ---
    // Ceilings and floor thorns form a wicked high-speed chicane
    addHazard(std::make_unique<Hazard>(Vector2{ 1860.0f, 400.0f }, Vector2{ 500.0f, 40.0f }, HazardType::THORN_CLUSTER));
    addHazard(std::make_unique<Hazard>(Vector2{ 1980.0f, 20.0f }, Vector2{ 350.0f, 40.0f }, HazardType::THORN_CLUSTER));

    // First S-Turn Anchor Node (Whip 1)
    addAnchor(std::make_unique<AnchorNode>(Vector2{ 1980.0f, 260.0f }, AnchorType::GLOWING_MUSHROOM, 150.0f));
    // Second S-Turn Anchor Node (Whip 2 - Sturdy Branch Knot)
    addAnchor(std::make_unique<AnchorNode>(Vector2{ 2220.0f, 140.0f }, AnchorType::STURDY_BRANCH_KNOT, 150.0f));

    addCollectable(std::make_unique<Collectable>(Vector2{ 2100.0f, 180.0f }, 250));

    // --- ZONE 7: Predatory Flora Gate & Final High Canopy Vine ---
    // Predatory Flora barrier (requires high speed smash)
    addBarrier(std::make_unique<BreakableBarrier>(Vector2{ 2440.0f, 100.0f }, Vector2{ 30.0f, 150.0f }, BarrierType::PREDATORY_FLORA));

    // High Canopy Slingshot Vine
    addVine(std::make_unique<Vine>(Vector2{ 2680.0f, 40.0f }, 175.0f));

    // Extra Anchor Node for extreme momentum recovery
    addAnchor(std::make_unique<AnchorNode>(Vector2{ 2940.0f, 130.0f }, AnchorType::GLOWING_MUSHROOM, 160.0f));

    // --- ZONE 8: Ancient High-Canopy Sanctuary (Goal Haven) ---
    m_goalSanctuary = { 3250.0f, 160.0f };
    // Massive sacred Banyan canopy sanctuary platform safe from the spore rot
    addPlatform(std::make_unique<Platform>(Vector2{ 3180.0f, 210.0f }, Vector2{ 360.0f, 60.0f }, PlatformType::SOLID));
    addPlatform(std::make_unique<Platform>(Vector2{ 3280.0f, 120.0f }, Vector2{ 160.0f, 20.0f }, PlatformType::ONE_WAY));
    addCollectable(std::make_unique<Collectable>(Vector2{ 3360.0f, 70.0f }, 500));
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

        if (PhysicsEngine::CheckAABB(playerBounds, h->GetBounds())) {
            // Hit thorns: burst & reset
            Vector2 pCenter = { playerBounds.x + playerBounds.width * 0.5f, playerBounds.y + playerBounds.height * 0.5f };
            ParticleSystem::GetInstance().EmitBurst(pCenter, 25, Color{ 220, 20, 60, 255 }, 120.0f, 320.0f, 0.4f);
            AddScreenShake(12.0f);
            m_player->Reset(m_spawnPos);
            m_rotWave.Reset(520.0f);
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
        return;
    }

    // 3. Fall out of world boundary
    if (m_player->GetPosition().y > 800.0f) {
        m_player->Reset(m_spawnPos);
        m_rotWave.Reset(520.0f);
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

    // Quick Restart
    if (IsKeyPressed(KEY_R)) {
        if (m_player) {
            m_player->Reset(m_spawnPos);
            m_rotWave.Reset(520.0f);
            m_reachedSanctuary = false;
        }
    }

    // 1. Update Vines (pendulum physics)
    for (auto* vine : m_vines) {
        if (vine && vine->IsActive()) {
            vine->Update(dt);
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
            h->Update(dt);
        }
    }

    // 5. Update Player with Brachiation & Tail Snap
    if (m_player && m_player->IsActive()) {
        m_player->HandleInput(dt, m_vines, m_anchorNodes);

        // When free falling / jumping / sliding, physics engine handles kinematic collision
        PlayerMovementState state = m_player->GetMovementState();
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
        if (m_player->GetPosition().x >= m_goalSanctuary.x && m_player->GetPosition().y <= m_goalSanctuary.y + 60.0f) {
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

void PlayState::Draw() {
    // 1. Draw Canopy Parallax Backdrops
    DrawBackground();

    // 2. Draw World Scene
    BeginMode2D(m_camera);

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

    // 3. Draw Canopy Snap Kinetic HUD
    DrawHUD();
}

void PlayState::DrawHUD() const {
    if (!m_player) return;

    // --- TOP-LEFT STATUS & GAUGES ---
    DrawRectangle(14, 14, 380, 105, Color{ 14, 18, 26, 235 });
    DrawRectangleLines(14, 14, 380, 105, Color{ 45, 60, 85, 255 });

    // Title & Logo
    DrawText("CANOPY SNAP", 26, 22, 16, Color{ 0, 255, 220, 255 });

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
    DrawText(TextFormat("VELOCITY: %.0f px/s", currentSpeed), 26, 46, 12, (currentSpeed > 450.0f) ? Color{ 255, 215, 0, 255 } : LIGHTGRAY);

    // Speed bar
    float speedRatio = std::clamp(currentSpeed / 900.0f, 0.0f, 1.0f);
    DrawRectangle(150, 48, 170, 8, Color{ 30, 35, 48, 255 });
    DrawRectangle(150, 48, static_cast<int>(170.0f * speedRatio), 8, (currentSpeed > 500.0f) ? Color{ 255, 140, 0, 255 } : Color{ 52, 152, 219, 255 });

    // POUNCE METER GAUGE
    float pounceMeter = m_player->GetPounceMeter();
    DrawText("POUNCE METER:", 26, 68, 12, (pounceMeter >= 25.0f) ? Color{ 255, 165, 0, 255 } : GRAY);

    DrawRectangle(150, 68, 170, 14, Color{ 30, 35, 48, 255 });
    Color pounceColor = (pounceMeter >= 25.0f) ? Color{ 255, 140, 0, 255 } : Color{ 160, 100, 30, 200 };
    DrawRectangle(150, 68, static_cast<int>(170.0f * (pounceMeter / 100.0f)), 14, pounceColor);
    DrawRectangleLines(150, 68, 170, 14, Color{ 70, 80, 105, 255 });

    if (pounceMeter >= 25.0f) {
        DrawText("READY [Q / F / Shift]", 162, 70, 10, WHITE);
    }

    // Run Timer & Score
    DrawText(TextFormat("TIME: %.1fs  |  SCORE: %d", m_runTimer, m_score), 26, 96, 11, GOLD);

    // --- BOTTOM CONTROLS QUICK BAR ---
    DrawRectangle(14, GetScreenHeight() - 38, 540, 26, Color{ 14, 18, 26, 215 });
    DrawText("[Space/LMB] Grab Vine & Brachiate  |  [E/RMB/Shift] Tail Snap  |  [Q/F] Pounce Smash  |  [R] Reset",
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
        DrawText("Press [R] to race through the canopy again!", GetScreenWidth() / 2 - 150, GetScreenHeight() / 2 + 35, 12, Color{ 255, 215, 0, 255 });
    }
}
