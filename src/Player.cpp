#include "Player.h"
#include "ResourceManager.h"
#include "Vine.h"
#include "AnchorNode.h"
#include "ParticleSystem.h"
#include <cmath>
#include <algorithm>

Player::Player(Vector2 position)
    : Entity(position, { 26.0f, 32.0f }, EntityType::PLAYER),
      m_state(PlayerMovementState::AIRBORNE),
      m_moveSpeed(280.0f),
      m_acceleration(1600.0f),
      m_deceleration(1400.0f),
      m_airControl(0.85f),
      m_jumpForce(-440.0f),
      m_currentVine(nullptr),
      m_tailNode(nullptr),
      m_targetAnchorNode(nullptr),
      m_tailAngle(0.0f),
      m_tailAngularVelocity(0.0f),
      m_tailRadius(80.0f),
      m_tailTimer(0.0f),
      m_tailMaxDuration(0.22f),
      m_pounceMeter(30.0f),
      m_pounceTimer(0.0f),
      m_pounceDirection({ 1.0f, 0.0f }),
      m_wallClingTimer(0.0f),
      m_coyoteTime(0.12f),
      m_coyoteTimer(0.0f),
      m_jumpBufferTime(0.12f),
      m_jumpBufferTimer(0.0f),
      m_facingRight(true),
      m_sweetSpotFeedbackTimer(0.0f),
      m_tailTipPosition(position),
      m_tailWhipAnim(0.0f),
      m_currentAnim(nullptr) {

    // Retrieve master character spritesheet (7 cols x 6 rows of 128x192 cells)
    Texture2D& playerTex = ResourceManager::GetInstance().GetTexture("player");

    // Row 0: Idle (5 frames, 7 fps)
    m_animIdle      = Animation(&playerTex, 5,  7.0f, 0, 0, 128, 192, true);
    // Row 1: Run (7 frames, 12 fps)
    m_animRun       = Animation(&playerTex, 7, 12.0f, 0, 1, 128, 192, true);
    // Row 2: Jump (7 frames, 10 fps)
    m_animJump      = Animation(&playerTex, 7, 10.0f, 0, 2, 128, 192, false);
    // Row 3: Swing Brachiation (7 frames, 12 fps)
    m_animSwing     = Animation(&playerTex, 7, 12.0f, 0, 3, 128, 192, true);
    // Row 4: Tail Hook Pivot (7 frames, 14 fps)
    m_animTailHook  = Animation(&playerTex, 7, 14.0f, 0, 4, 128, 192, true);
    // Row 5: Wall Cling (6 frames, 8 fps)
    m_animWallCling = Animation(&playerTex, 6,  8.0f, 0, 5, 128, 192, true);

    m_currentAnim = &m_animIdle;
}

void Player::Reset(Vector2 spawnPos) {
    if (m_currentVine) {
        m_currentVine->DetachPlayer();
        m_currentVine = nullptr;
    }
    m_position = spawnPos;
    m_velocity = { 0.0f, 0.0f };
    m_state = PlayerMovementState::AIRBORNE;
    m_pounceTimer = 0.0f;
    m_pounceMeter = 30.0f;
    m_tailNode = nullptr;
    m_targetAnchorNode = nullptr;
    m_isGrounded = false;
    m_wasGrounded = false;
}

float Player::GetSpeed() const {
    return Vector2Length(m_velocity);
}

bool Player::TryGrabVine(const std::vector<Vine*>& vines) {
    Vector2 center = { m_position.x + m_size.x * 0.5f, m_position.y + m_size.y * 0.5f };

    for (auto* vine : vines) {
        if (!vine || !vine->IsActive()) continue;

        if (vine->CanGrab(center, 44.0f)) {
            m_currentVine = vine;
            m_currentVine->AttachPlayer(m_velocity);
            m_state = PlayerMovementState::BRACHIATING;
            ParticleSystem::GetInstance().EmitBurst(center, 10, Color{ 46, 204, 113, 255 }, 60.0f, 180.0f, 0.25f);
            return true;
        }
    }
    return false;
}

void Player::ReleaseVine() {
    if (!m_currentVine) return;

    bool sweetSpot = false;
    m_velocity = m_currentVine->CalculateReleaseVelocity(&sweetSpot);
    m_currentVine->DetachPlayer();
    m_currentVine = nullptr;
    m_state = PlayerMovementState::AIRBORNE;

    Vector2 center = { m_position.x + m_size.x * 0.5f, m_position.y + m_size.y * 0.5f };

    if (sweetSpot) {
        m_sweetSpotFeedbackTimer = 0.4f;
        m_pounceMeter = std::min(100.0f, m_pounceMeter + 25.0f);
        ParticleSystem::GetInstance().EmitBurst(center, 22, Color{ 255, 215, 0, 255 }, 120.0f, 340.0f, 0.4f);
    } else {
        ParticleSystem::GetInstance().EmitBurst(center, 8, Color{ 200, 240, 200, 200 }, 60.0f, 180.0f, 0.2f);
    }
}

bool Player::TryTailSnap(const std::vector<AnchorNode*>& anchorNodes) {
    Vector2 center = { m_position.x + m_size.x * 0.5f, m_position.y + m_size.y * 0.5f };

    AnchorNode* bestNode = nullptr;
    float bestDistSq = 1e9f;

    for (auto* node : anchorNodes) {
        if (!node || !node->IsActive() || !node->CanSnap()) continue;

        if (node->IsInRange(center)) {
            float dx = center.x - node->GetCenter().x;
            float dy = center.y - node->GetCenter().y;
            float distSq = dx * dx + dy * dy;
            if (distSq < bestDistSq) {
                bestDistSq = distSq;
                bestNode = node;
            }
        }
    }

    if (!bestNode) return false;

    // Release current vine if any
    if (m_currentVine) {
        m_currentVine->DetachPlayer();
        m_currentVine = nullptr;
    }

    m_tailNode = bestNode;
    m_tailNode->TriggerSnap();

    Vector2 nodePos = m_tailNode->GetCenter();
    Vector2 rel = Vector2Subtract(center, nodePos);
    m_tailRadius = std::clamp(Vector2Length(rel), 45.0f, 130.0f);
    m_tailAngle = std::atan2(rel.y, rel.x);

    // Maintain momentum: determine angular velocity from incoming linear speed
    float speed = std::max(GetSpeed(), 520.0f);

    // Cross product (rel x vel) determines rotation direction:
    float cross = rel.x * m_velocity.y - rel.y * m_velocity.x;
    float rotDir = (cross >= 0.0f) ? 1.0f : -1.0f;

    m_tailAngularVelocity = rotDir * (speed / m_tailRadius);
    m_tailTimer = 0.0f;
    m_tailMaxDuration = 0.22f; // Snappy whip arc

    m_state = PlayerMovementState::TAIL_PIVOTING;

    // Visual snap line
    ParticleSystem::GetInstance().EmitTailSnap(center, nodePos);
    return true;
}

void Player::ReleaseTailSnap() {
    if (!m_tailNode) return;

    // Compute launch vector perpendicular to radius
    float speed = std::abs(m_tailAngularVelocity * m_tailRadius) * 1.15f; // 15% momentum kick
    float launchAngle = m_tailAngle + ((m_tailAngularVelocity > 0.0f) ? (PI * 0.5f) : (-PI * 0.5f));

    m_velocity = {
        std::cos(launchAngle) * speed,
        std::sin(launchAngle) * speed
    };

    Vector2 center = { m_position.x + m_size.x * 0.5f, m_position.y + m_size.y * 0.5f };
    ParticleSystem::GetInstance().EmitBurst(center, 18, Color{ 0, 255, 230, 255 }, 120.0f, 320.0f, 0.35f);

    m_pounceMeter = std::min(100.0f, m_pounceMeter + 20.0f);
    m_tailNode = nullptr;
    m_state = PlayerMovementState::AIRBORNE;
}

bool Player::TryPounce() {
    // Requires at least 25% meter
    if (m_pounceMeter < 25.0f) return false;

    // Deduct cost
    m_pounceMeter -= 25.0f;

    // Direction based on user input or velocity or facing
    Vector2 dir = { 0.0f, 0.0f };
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) dir.x += 1.0f;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))  dir.x -= 1.0f;
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))    dir.y -= 1.0f;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))  dir.y += 1.0f;

    if (Vector2Length(dir) < 0.1f) {
        dir = { m_facingRight ? 1.0f : -1.0f, -0.2f };
    }
    dir = Vector2Normalize(dir);

    m_pounceDirection = dir;
    m_pounceTimer = 0.24f;
    m_velocity = Vector2Scale(dir, 850.0f); // High-speed kinetic burst

    if (m_currentVine) {
        m_currentVine->DetachPlayer();
        m_currentVine = nullptr;
    }
    if (m_tailNode) {
        m_tailNode = nullptr;
    }

    m_state = PlayerMovementState::POUNCING;

    Vector2 center = { m_position.x + m_size.x * 0.5f, m_position.y + m_size.y * 0.5f };
    ParticleSystem::GetInstance().EmitBurst(center, 24, Color{ 255, 140, 0, 255 }, 150.0f, 400.0f, 0.4f);
    return true;
}

void Player::HandleInput(float dt, const std::vector<Vine*>& vines, const std::vector<AnchorNode*>& anchorNodes) {
    // Update nearest anchor highlight
    Vector2 center = { m_position.x + m_size.x * 0.5f, m_position.y + m_size.y * 0.5f };
    m_targetAnchorNode = nullptr;
    float bestDistSq = 1e9f;

    for (auto* node : anchorNodes) {
        if (!node || !node->IsActive()) continue;
        node->SetHighlighted(false);
        if (node->IsInRange(center) && node->CanSnap()) {
            float dx = center.x - node->GetCenter().x;
            float dy = center.y - node->GetCenter().y;
            float dsq = dx * dx + dy * dy;
            if (dsq < bestDistSq) {
                bestDistSq = dsq;
                m_targetAnchorNode = node;
            }
        }
    }
    if (m_targetAnchorNode) {
        m_targetAnchorNode->SetHighlighted(true);
    }

    // 1. BRACHIATING STATE
    if (m_state == PlayerMovementState::BRACHIATING) {
        float pumpInput = 0.0f;
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) pumpInput += 1.0f;
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))  pumpInput -= 1.0f;

        if (pumpInput > 0.0f) m_facingRight = true;
        else if (pumpInput < 0.0f) m_facingRight = false;

        if (m_currentVine) {
            m_currentVine->ApplyPumpTorque(pumpInput, dt);
        }

        // Release vine on key release
        if (IsKeyReleased(KEY_SPACE) || IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            ReleaseVine();
        }

        // Can also trigger pounce directly from swing
        if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_F) || IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
            TryPounce();
        }
        return;
    }

    // 2. TAIL PIVOTING STATE
    if (m_state == PlayerMovementState::TAIL_PIVOTING) {
        m_tailTimer += dt;
        m_tailAngle += m_tailAngularVelocity * dt;

        // Visual tail whip particles
        Vector2 curPos = { m_position.x + m_size.x * 0.5f, m_position.y + m_size.y * 0.5f };
        ParticleSystem::GetInstance().EmitTrail(curPos, m_velocity, Color{ 0, 255, 230, 220 }, 5.0f, 0.2f);

        // Whip completed or player released snap
        if (m_tailTimer >= m_tailMaxDuration || IsKeyReleased(KEY_E) || IsMouseButtonReleased(MOUSE_BUTTON_RIGHT) || IsKeyReleased(KEY_LEFT_SHIFT)) {
            ReleaseTailSnap();
        }
        return;
    }

    // 3. POUNCING STATE
    if (m_state == PlayerMovementState::POUNCING) {
        m_pounceTimer -= dt;
        m_velocity = Vector2Scale(m_pounceDirection, 850.0f);

        // Ghost streak particles
        Vector2 curPos = { m_position.x + m_size.x * 0.5f, m_position.y + m_size.y * 0.5f };
        ParticleSystem::GetInstance().EmitTrail(curPos, m_velocity, Color{ 255, 140, 0, 240 }, 6.0f, 0.25f);

        if (m_pounceTimer <= 0.0f) {
            m_state = PlayerMovementState::AIRBORNE;
        }
        return;
    }

    // 4. AIRBORNE / WALL_SLIDING INPUTS
    float moveInput = 0.0f;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) moveInput += 1.0f;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))  moveInput -= 1.0f;

    if (moveInput > 0.0f) m_facingRight = true;
    else if (moveInput < 0.0f) m_facingRight = false;

    // Check Wall Cling on vertical bark
    if (!m_isGrounded && (m_isWallLeft || m_isWallRight)) {
        m_state = PlayerMovementState::WALL_SLIDING;
        // Damp vertical fall speed along bark
        if (m_velocity.y > 60.0f) {
            m_velocity.y = 60.0f;
        }

        // Wall Jump
        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            float wallDir = m_isWallLeft ? 1.0f : -1.0f;
            m_velocity.x = wallDir * 380.0f;
            m_velocity.y = m_jumpForce * 0.95f;
            m_facingRight = (wallDir > 0.0f);
            m_state = PlayerMovementState::AIRBORNE;
            ParticleSystem::GetInstance().EmitBurst(center, 12, Color{ 180, 130, 80, 255 }, 80.0f, 220.0f, 0.3f);
        }
    } else if (m_state == PlayerMovementState::WALL_SLIDING) {
        m_state = PlayerMovementState::AIRBORNE;
    }

    // Horizontal Movement
    float currentAccel = m_isGrounded ? m_acceleration : (m_acceleration * m_airControl);
    float currentDecel = m_isGrounded ? m_deceleration : (m_deceleration * m_airControl);

    if (moveInput != 0.0f) {
        float targetSpeed = moveInput * m_moveSpeed;
        if (m_velocity.x < targetSpeed) {
            m_velocity.x = std::min(m_velocity.x + currentAccel * dt, targetSpeed);
        } else if (m_velocity.x > targetSpeed) {
            m_velocity.x = std::max(m_velocity.x - currentAccel * dt, targetSpeed);
        }
    } else {
        if (m_velocity.x > 0.0f) {
            m_velocity.x = std::max(0.0f, m_velocity.x - currentDecel * dt);
        } else if (m_velocity.x < 0.0f) {
            m_velocity.x = std::min(0.0f, m_velocity.x + currentDecel * dt);
        }
    }

    // Grab Vine with Space or Left Mouse Click
    if (IsKeyDown(KEY_SPACE) || IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        TryGrabVine(vines);
    }

    // Tail Snap with Right Mouse or E or Left Shift
    if (IsKeyPressed(KEY_E) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || IsKeyPressed(KEY_LEFT_SHIFT)) {
        TryTailSnap(anchorNodes);
    }

    // Pounce cash-in
    if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_F) || IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
        TryPounce();
    }
}

void Player::Update(float dt) {
    if (m_sweetSpotFeedbackTimer > 0.0f) {
        m_sweetSpotFeedbackTimer -= dt;
    }

    // 1. BRACHIATING: Follow vine tip
    if (m_state == PlayerMovementState::BRACHIATING && m_currentVine) {
        Vector2 tip = m_currentVine->GetTipPosition();
        m_position = { tip.x - m_size.x * 0.5f, tip.y };
        float vTangent = m_currentVine->GetAngularVelocity() * m_currentVine->GetLength();
        m_velocity = {
            std::cos(m_currentVine->GetAngle()) * vTangent,
            -std::sin(m_currentVine->GetAngle()) * vTangent
        };

        // Charge pounce meter while swinging fast
        float speed = std::abs(vTangent);
        if (speed > 350.0f) {
            m_pounceMeter = std::min(100.0f, m_pounceMeter + dt * 35.0f);
        }
    }
    // 2. TAIL PIVOTING: Follow anchor orbital arc
    else if (m_state == PlayerMovementState::TAIL_PIVOTING && m_tailNode) {
        Vector2 nPos = m_tailNode->GetCenter();
        m_position = {
            nPos.x + std::cos(m_tailAngle) * m_tailRadius - m_size.x * 0.5f,
            nPos.y + std::sin(m_tailAngle) * m_tailRadius - m_size.y * 0.5f
        };
        float speed = std::abs(m_tailAngularVelocity * m_tailRadius);
        float tangAngle = m_tailAngle + ((m_tailAngularVelocity > 0.0f) ? (PI * 0.5f) : (-PI * 0.5f));
        m_velocity = { std::cos(tangAngle) * speed, std::sin(tangAngle) * speed };
    }
    // 3. AIRBORNE / POUNCING
    else {
        float speed = GetSpeed();
        if (speed > 420.0f) {
            m_pounceMeter = std::min(100.0f, m_pounceMeter + dt * 25.0f);
            // Speed trail
            Vector2 curCenter = { m_position.x + m_size.x * 0.5f, m_position.y + m_size.y * 0.5f };
            ParticleSystem::GetInstance().EmitTrail(curCenter, m_velocity, Color{ 0, 200, 255, 180 }, 4.0f, 0.2f);
        }
    }

    UpdateTailPhysics(dt);
    UpdateAnimations(dt);
}

void Player::UpdateAnimations(float dt) {
    if (m_state == PlayerMovementState::BRACHIATING) {
        m_currentAnim = &m_animSwing;
        if (m_currentVine) {
            // Map the vine pendulum swing angle (-1.2 rad to +1.2 rad) to frame 0..6
            float angle = m_currentVine->GetAngle();
            float norm = (angle + 1.2f) / 2.4f;
            int frame = std::clamp(static_cast<int>(norm * 7.0f), 0, 6);
            m_animSwing.SetCurrentFrame(frame);
        } else {
            m_animSwing.Update(dt);
        }
    } else if (m_state == PlayerMovementState::TAIL_PIVOTING) {
        m_currentAnim = &m_animTailHook;
        m_animTailHook.Update(dt);
    } else if (m_state == PlayerMovementState::WALL_SLIDING) {
        m_currentAnim = &m_animWallCling;
        m_animWallCling.Update(dt);
    } else if (m_state == PlayerMovementState::POUNCING) {
        m_currentAnim = &m_animJump;
        m_animJump.SetCurrentFrame(3); // Aerodynamic mid-air leap pose
    } else if (!m_isGrounded) {
        m_currentAnim = &m_animJump;
        // Map jump frame to vertical velocity
        if (m_velocity.y < -180.0f) {
            m_animJump.SetCurrentFrame(1); // Ascending rocket
        } else if (m_velocity.y < 80.0f) {
            m_animJump.SetCurrentFrame(3); // Apex
        } else {
            m_animJump.SetCurrentFrame(5); // Falling
        }
    } else {
        // Grounded
        if (std::abs(m_velocity.x) > 15.0f) {
            m_currentAnim = &m_animRun;
            float runFps = std::clamp(std::abs(m_velocity.x) * 0.05f, 6.0f, 18.0f);
            m_animRun.SetFrameSpeed(runFps);
            m_animRun.Update(dt);
        } else {
            m_currentAnim = &m_animIdle;
            m_animIdle.Update(dt);
        }
    }
}

void Player::UpdateTailPhysics(float dt) {
    Vector2 hipPos = { m_position.x + (m_facingRight ? 4.0f : m_size.x - 4.0f), m_position.y + 20.0f };

    if (m_state == PlayerMovementState::TAIL_PIVOTING && m_tailNode) {
        m_tailTipPosition = m_tailNode->GetCenter();
    } else {
        // Natural prehensile tail trailing
        Vector2 targetTip = {
            hipPos.x + (m_facingRight ? -16.0f : 16.0f) - m_velocity.x * 0.03f,
            hipPos.y - 8.0f - m_velocity.y * 0.03f
        };
        m_tailTipPosition.x += (targetTip.x - m_tailTipPosition.x) * 12.0f * dt;
        m_tailTipPosition.y += (targetTip.y - m_tailTipPosition.y) * 12.0f * dt;
    }
}

void Player::Draw() {
    Vector2 center = { m_position.x + m_size.x * 0.5f, m_position.y + m_size.y * 0.5f };
    const float scale = 0.42f;
    const float spriteW = 128.0f * scale;
    const float spriteH = 192.0f * scale;

    float drawX = center.x - (64.0f * scale);
    float drawY = (m_position.y + m_size.y) - (175.0f * scale);
    bool flipX = !m_facingRight;

    // Tail snap cyan tether whip
    if (m_state == PlayerMovementState::TAIL_PIVOTING && m_tailNode) {
        Vector2 hipPos = { m_position.x + (m_facingRight ? 4.0f : m_size.x - 4.0f), m_position.y + 16.0f };
        Vector2 nodeCenter = m_tailNode->GetCenter();
        DrawLineEx(hipPos, nodeCenter, 4.0f, Color{ 0, 255, 230, 255 });
        DrawLineEx(hipPos, nodeCenter, 2.0f, WHITE);
    }

    // Special alignment for Brachiation swing
    if (m_state == PlayerMovementState::BRACHIATING && m_currentVine) {
        Vector2 vineTip = m_currentVine->GetTipPosition();
        drawX = vineTip.x - (64.0f * scale);
        drawY = vineTip.y - (14.0f * scale);
        // Face swing direction
        flipX = (m_currentVine->GetAngularVelocity() < 0.0f);
    }
    // Alignment for Wall Cling
    else if (m_state == PlayerMovementState::WALL_SLIDING) {
        flipX = m_isWallLeft; // cling face to bark
    }

    // Dynamic tint based on kinetic state
    Color tint = WHITE;
    if (m_state == PlayerMovementState::POUNCING) {
        tint = Color{ 255, 160, 40, 255 }; // Blazing pounce aura
    } else if (GetSpeed() > 500.0f) {
        tint = Color{ 210, 245, 255, 255 }; // Kinetic hyper-speed glint
    }

    // DRAW THE CHARACTER ANIMATION FROM ASSET SPRITESHEET
    if (m_currentAnim) {
        m_currentAnim->Draw({ drawX, drawY }, { spriteW, spriteH }, flipX, tint);
    }

    // Sweet spot launch prompt flash
    if (m_sweetSpotFeedbackTimer > 0.0f) {
        DrawText("PERFECT SNAP!", static_cast<int>(center.x - 45), static_cast<int>(m_position.y - 24), 11, Color{ 255, 215, 0, 255 });
    }
}

void Player::DrawDebug() const {
    DrawRectangleLinesEx(GetBounds(), 1.0f, GREEN);
    DrawText(TextFormat("Spd: %.0f", GetSpeed()), static_cast<int>(m_position.x), static_cast<int>(m_position.y - 12), 10, WHITE);
}
