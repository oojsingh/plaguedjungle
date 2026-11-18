#include "AnchorNode.h"
#include "ParticleSystem.h"
#include "ResourceManager.h"
#include <cmath>
#include <algorithm>

AnchorNode::AnchorNode(Vector2 position, AnchorType type, float radius, bool fragile)
    : Entity(position, { 32.0f, 32.0f }, EntityType::ANCHOR_NODE),
      m_anchorType(type),
      m_radius(radius),
      m_isHighlighted(false),
      m_cooldownTimer(0.0f),
      m_pulseTimer(0.0f),
      m_fragile(fragile),
      m_startPos(position),
      m_endPos(position),
      m_moveSpeed(0.0f),
      m_movingToEnd(true) {}

void AnchorNode::SetMovingPath(Vector2 endPosition, float speed) {
    m_endPos = endPosition;
    m_moveSpeed = speed;
}

bool AnchorNode::IsInRange(Vector2 playerCenter) const {
    float dx = playerCenter.x - m_position.x;
    float dy = playerCenter.y - m_position.y;
    return (dx * dx + dy * dy) <= (m_radius * m_radius);
}

void AnchorNode::TriggerSnap() {
    m_cooldownTimer = 0.45f;
    ParticleSystem::GetInstance().EmitBurst(m_position, 14, Color{ 0, 255, 230, 255 }, 80.0f, 250.0f, 0.35f);
    if (m_fragile) {
        Destroy();
    }
}

void AnchorNode::Update(float dt) {
    m_pulseTimer += dt;
    if (m_moveSpeed > 0.0f) {
        Vector2 target = m_movingToEnd ? m_endPos : m_startPos;
        Vector2 delta = Vector2Subtract(target, m_position);
        float distance = Vector2Length(delta);
        float step = m_moveSpeed * dt;
        if (distance <= step) {
            m_position = target;
            m_movingToEnd = !m_movingToEnd;
        } else {
            m_position = Vector2Add(m_position, Vector2Scale(Vector2Normalize(delta), step));
        }
    }
    if (m_cooldownTimer > 0.0f) {
        m_cooldownTimer -= dt;
    }
}

void AnchorNode::Draw() {
    float pulse = std::sin(m_pulseTimer * 4.0f) * 2.0f;
    float baseRadius = 14.0f + pulse;

    if (m_anchorType == AnchorType::GLOWING_MUSHROOM) {
        Texture2D& anchor = ResourceManager::GetInstance().GetTexture("snap_anchor");
        if (anchor.id != 0) {
            DrawCircleGradient(static_cast<int>(m_position.x), static_cast<int>(m_position.y),
                               baseRadius * 2.2f, Color{ 0, 240, 220, 70 }, BLANK);
            Rectangle source = { 0.0f, 0.0f, static_cast<float>(anchor.width), static_cast<float>(anchor.height) };
            Rectangle destination = { m_position.x - 17.0f, m_position.y - 20.0f, 34.0f, 40.0f };
            Color tint = m_fragile ? Color{ 255, 125, 125, 255 } : WHITE;
            DrawTexturePro(anchor, source, destination, { 0.0f, 0.0f }, 0.0f, tint);
        } else {
            DrawCircle(static_cast<int>(m_position.x), static_cast<int>(m_position.y), baseRadius, SKYBLUE);
        }
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
