#include "AnchorNode.h"
#include "ParticleSystem.h"
#include <cmath>
#include <algorithm>

AnchorNode::AnchorNode(Vector2 position, AnchorType type, float radius)
    : Entity(position, { 32.0f, 32.0f }, EntityType::ANCHOR_NODE),
      m_anchorType(type),
      m_radius(radius),
      m_isHighlighted(false),
      m_cooldownTimer(0.0f),
      m_pulseTimer(0.0f) {}

bool AnchorNode::IsInRange(Vector2 playerCenter) const {
    float dx = playerCenter.x - m_position.x;
    float dy = playerCenter.y - m_position.y;
    return (dx * dx + dy * dy) <= (m_radius * m_radius);
}

void AnchorNode::TriggerSnap() {
    m_cooldownTimer = 0.45f;
    ParticleSystem::GetInstance().EmitBurst(m_position, 14, Color{ 0, 255, 230, 255 }, 80.0f, 250.0f, 0.35f);
}

void AnchorNode::Update(float dt) {
    m_pulseTimer += dt;
    if (m_cooldownTimer > 0.0f) {
        m_cooldownTimer -= dt;
    }
}

void AnchorNode::Draw() {
    float pulse = std::sin(m_pulseTimer * 4.0f) * 2.0f;
    float baseRadius = 14.0f + pulse;

    if (m_anchorType == AnchorType::GLOWING_MUSHROOM) {
        // Glowing Bioluminescent Mushroom cap
        Color glowColor = m_isHighlighted ? Color{ 0, 255, 220, 255 } : Color{ 52, 152, 219, 220 };
        Color capColor  = m_isHighlighted ? Color{ 120, 255, 240, 255 } : Color{ 41, 128, 185, 255 };

        // Outer ambient glow ring
        DrawCircleGradient(static_cast<int>(m_position.x), static_cast<int>(m_position.y),
                           baseRadius * 2.2f, Color{ 0, 240, 220, 70 }, BLANK);

        // Mushroom stalk
        DrawRectangle(static_cast<int>(m_position.x - 3), static_cast<int>(m_position.y), 6, 12, Color{ 220, 240, 245, 200 });

        // Cap
        DrawCircleSector({ m_position.x, m_position.y }, baseRadius, 180.0f, 360.0f, 16, capColor);
        DrawCircleSectorLines({ m_position.x, m_position.y }, baseRadius, 180.0f, 360.0f, 16, glowColor);

        // Bioluminescent spore spots
        DrawCircle(static_cast<int>(m_position.x - 5), static_cast<int>(m_position.y - 6), 2.5f, WHITE);
        DrawCircle(static_cast<int>(m_position.x + 4), static_cast<int>(m_position.y - 7), 2.0f, WHITE);
        DrawCircle(static_cast<int>(m_position.x),     static_cast<int>(m_position.y - 11), 2.2f, WHITE);
    } else {
        // Sturdy Ancient Branch Knot
        Color knotColor = m_isHighlighted ? Color{ 241, 196, 15, 255 } : Color{ 130, 80, 40, 255 };
        DrawCircle(static_cast<int>(m_position.x), static_cast<int>(m_position.y), baseRadius, knotColor);
        DrawCircleLines(static_cast<int>(m_position.x), static_cast<int>(m_position.y), baseRadius + 3.0f, Color{ 80, 45, 20, 255 });
        DrawCircle(static_cast<int>(m_position.x), static_cast<int>(m_position.y), 4.0f, Color{ 245, 230, 150, 220 });
    }

    // If player is in range, draw interactive snap reticle
    if (m_isHighlighted && m_cooldownTimer <= 0.0f) {
        float reticleRadius = 26.0f + std::sin(m_pulseTimer * 8.0f) * 3.0f;
        Color reticleColor = Color{ 0, 255, 230, 220 };

        DrawCircleLines(static_cast<int>(m_position.x), static_cast<int>(m_position.y), reticleRadius, reticleColor);
        DrawCircleLines(static_cast<int>(m_position.x), static_cast<int>(m_position.y), reticleRadius + 4.0f, Color{ 0, 255, 230, 100 });

        // Crosshairs / Snap brackets
        float armLen = 8.0f;
        DrawLine(static_cast<int>(m_position.x - reticleRadius - armLen), static_cast<int>(m_position.y),
                 static_cast<int>(m_position.x - reticleRadius + 2), static_cast<int>(m_position.y), reticleColor);
        DrawLine(static_cast<int>(m_position.x + reticleRadius - 2), static_cast<int>(m_position.y),
                 static_cast<int>(m_position.x + reticleRadius + armLen), static_cast<int>(m_position.y), reticleColor);
        DrawLine(static_cast<int>(m_position.x), static_cast<int>(m_position.y - reticleRadius - armLen),
                 static_cast<int>(m_position.x), static_cast<int>(m_position.y - reticleRadius + 2), reticleColor);
        DrawLine(static_cast<int>(m_position.x), static_cast<int>(m_position.y + reticleRadius - 2),
                 static_cast<int>(m_position.x), static_cast<int>(m_position.y + reticleRadius + armLen), reticleColor);

        // Prompt text
        DrawText("TAIL SNAP", static_cast<int>(m_position.x - 30), static_cast<int>(m_position.y - 36), 10, Color{ 0, 255, 230, 240 });
    }
}

void AnchorNode::DrawDebug() const {
    DrawCircleLines(static_cast<int>(m_position.x), static_cast<int>(m_position.y), m_radius, SKYBLUE);
}
