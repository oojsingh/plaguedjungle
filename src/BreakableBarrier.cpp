#include "BreakableBarrier.h"
#include "ParticleSystem.h"
#include <cmath>
#include <cstdlib>

BreakableBarrier::BreakableBarrier(Vector2 position, Vector2 size, BarrierType type)
    : Entity(position, size, EntityType::BARRIER),
      m_barrierType(type),
      m_isBroken(false),
      m_shakeTimer(0.0f) {}

void BreakableBarrier::Break() {
    if (m_isBroken) return;
    m_isBroken = true;
    m_active = false;

    Vector2 center = { m_position.x + m_size.x * 0.5f, m_position.y + m_size.y * 0.5f };
    if (m_barrierType == BarrierType::BRITTLE_WOOD) {
        ParticleSystem::GetInstance().EmitWoodSplinters(center, 30);
    } else {
        ParticleSystem::GetInstance().EmitBurst(center, 25, Color{ 142, 68, 173, 255 }, 120.0f, 340.0f, 0.45f);
    }
}

void BreakableBarrier::Update(float dt) {
    if (m_shakeTimer > 0.0f) {
        m_shakeTimer -= dt;
    }
}

void BreakableBarrier::Draw() {
    if (m_isBroken) return;

    Rectangle bounds = GetBounds();

    if (m_barrierType == BarrierType::BRITTLE_WOOD) {
        // Brittle cracked ancient logs
        DrawRectangleRec(bounds, Color{ 101, 67, 33, 255 });
        DrawRectangleLinesEx(bounds, 2.0f, Color{ 70, 45, 20, 255 });

        // Cracks & splinter lines
        float step = 14.0f;
        for (float y = bounds.y + 6.0f; y < bounds.y + bounds.height - 4.0f; y += step) {
            DrawLineEx({ bounds.x + 4.0f, y }, { bounds.x + bounds.width - 4.0f, y + 5.0f }, 2.0f, Color{ 50, 30, 15, 255 });
            // Glowing weak-point fracture
            DrawLineEx({ bounds.x + 8.0f, y + 2.0f }, { bounds.x + bounds.width - 8.0f, y + 3.0f }, 1.0f, Color{ 230, 126, 34, 180 });
        }
    } else {
        // Predatory Flora (hostile carnivorous vines / thorn cluster barrier)
        DrawRectangleRec(bounds, Color{ 75, 0, 130, 240 });
        DrawRectangleLinesEx(bounds, 2.0f, Color{ 138, 43, 226, 255 });

        // Pulsing thorn spines
        for (float x = bounds.x + 6.0f; x < bounds.x + bounds.width; x += 16.0f) {
            DrawTriangle({ x, bounds.y }, { x + 8.0f, bounds.y - 10.0f }, { x + 16.0f, bounds.y }, Color{ 220, 20, 60, 255 });
            DrawTriangle({ x, bounds.y + bounds.height }, { x + 16.0f, bounds.y + bounds.height }, { x + 8.0f, bounds.y + bounds.height + 10.0f }, Color{ 220, 20, 60, 255 });
        }
    }
}

void BreakableBarrier::DrawDebug() const {
    if (!m_isBroken) {
        DrawRectangleLinesEx(GetBounds(), 1.0f, ORANGE);
    }
}
