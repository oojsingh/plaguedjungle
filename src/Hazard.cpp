#include "Hazard.h"
#include <cmath>

Hazard::Hazard(Vector2 position, Vector2 size, HazardType type)
    : Entity(position, size, EntityType::HAZARD),
      m_hazardType(type),
      m_animTimer(0.0f) {}

void Hazard::Update(float dt) {
    m_animTimer += dt;
}

void Hazard::Draw() {
    Rectangle bounds = GetBounds();

    if (m_hazardType == HazardType::THORN_CLUSTER) {
        // Jagged bramble base
        DrawRectangleRec(bounds, Color{ 38, 20, 28, 240 });
        DrawRectangleLinesEx(bounds, 1.5f, Color{ 80, 25, 45, 255 });

        // Jagged thorns protruding out
        float spikeWidth = 14.0f;
        for (float x = bounds.x; x < bounds.x + bounds.width - 4.0f; x += spikeWidth) {
            float pulse = std::sin(m_animTimer * 5.0f + x * 0.1f) * 2.0f;
            Vector2 p1 = { x, bounds.y };
            Vector2 p2 = { x + spikeWidth * 0.5f, bounds.y - 12.0f + pulse };
            Vector2 p3 = { x + spikeWidth, bounds.y };
            DrawTriangle(p1, p2, p3, Color{ 192, 57, 43, 255 });

            // Thorn barb tip highlight
            DrawCircleV(p2, 1.5f, Color{ 255, 120, 100, 255 });
        }
    } else {
        // Spore Vent
        DrawRectangleRec(bounds, Color{ 44, 62, 80, 255 });
        DrawCircle(static_cast<int>(bounds.x + bounds.width * 0.5f), static_cast<int>(bounds.y + bounds.height * 0.5f),
                   bounds.height * 0.4f, Color{ 142, 68, 173, 255 });
    }
}

void Hazard::DrawDebug() const {
    DrawRectangleLinesEx(GetBounds(), 1.0f, RED);
}
