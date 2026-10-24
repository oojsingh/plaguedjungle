#include "RotWave.h"
#include "ParticleSystem.h"
#include <cmath>
#include <algorithm>

RotWave::RotWave(float initialGroundY, float riseSpeed)
    : m_currentY(initialGroundY),
      m_startY(initialGroundY),
      m_riseSpeed(riseSpeed),
      m_currentX(-300.0f),
      m_creepSpeedX(45.0f),
      m_waveTimer(0.0f) {}

void RotWave::Reset(float groundY) {
    m_startY = groundY;
    m_currentY = groundY;
    m_currentX = -300.0f;
    m_waveTimer = 0.0f;
}

void RotWave::Update(float dt, Vector2 playerPos) {
    m_waveTimer += dt;

    // Slowly rise up towards a maximum canopy threshold (e.g. up to Y = 160)
    if (m_currentY > 160.0f) {
        m_currentY -= m_riseSpeed * dt;
    }

    // Horizontal pursuit: keeps creeping forward, but never overtakes player instantly
    m_currentX += m_creepSpeedX * dt;
    if (playerPos.x - m_currentX > 900.0f) {
        // Soft leash so rot stays threatening
        m_currentX = playerPos.x - 900.0f;
    }

    // Emit toxic spore bubbles along the visible surface
    float emitStartX = playerPos.x - 500.0f;
    float emitWidth = 1000.0f;
    ParticleSystem::GetInstance().EmitSporeFog({ emitStartX, m_currentY }, emitWidth);
}

bool RotWave::IsSubmerged(Vector2 playerPos, Vector2 playerSize) const {
    // Check if player feet touch the rising toxic spore ground
    if (playerPos.y + playerSize.y >= m_currentY) {
        return true;
    }
    // Check if player got caught in trailing rot wall
    if (playerPos.x + playerSize.x <= m_currentX) {
        return true;
    }
    return false;
}

void RotWave::Draw(const Camera2D& camera) const {
    float viewLeft = camera.target.x - 800.0f;
    float viewRight = camera.target.x + 800.0f;
    float viewBottom = m_currentY + 600.0f;

    // 1. Draw Toxic Spore Deep Ground Mass
    Rectangle rotGround = { viewLeft, m_currentY, viewRight - viewLeft, viewBottom - m_currentY };
    DrawRectangleRec(rotGround, Color{ 34, 15, 45, 230 });

    // 2. Undulating toxic foam/mist waves along the top surface
    int steps = 50;
    float stepWidth = (viewRight - viewLeft) / steps;
    for (int i = 0; i < steps; ++i) {
        float x1 = viewLeft + i * stepWidth;
        float x2 = x1 + stepWidth;
        float wave1 = std::sin(m_waveTimer * 3.0f + x1 * 0.02f) * 12.0f;
        float wave2 = std::sin(m_waveTimer * 3.0f + x2 * 0.02f) * 12.0f;

        Vector2 p1 = { x1, m_currentY + wave1 };
        Vector2 p2 = { x2, m_currentY + wave2 };
        Vector2 p3 = { x2, m_currentY + 40.0f };
        Vector2 p4 = { x1, m_currentY + 40.0f };

        // Bioluminescent poisonous crest
        DrawLineEx(p1, p2, 4.0f, Color{ 142, 68, 173, 255 });
        DrawLineEx({ p1.x, p1.y - 2.0f }, { p2.x, p2.y - 2.0f }, 2.0f, Color{ 46, 204, 113, 200 });
    }

    // 3. Trailing Rot Wall on the left
    Rectangle rotWall = { m_currentX - 500.0f, -500.0f, 500.0f, 1500.0f };
    DrawRectangleRec(rotWall, Color{ 25, 10, 35, 240 });
    DrawLineEx({ m_currentX, -500.0f }, { m_currentX, 1000.0f }, 6.0f, Color{ 155, 89, 182, 220 });
}
