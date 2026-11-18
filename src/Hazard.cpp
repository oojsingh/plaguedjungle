#include "Hazard.h"
#include "ResourceManager.h"
#include <cmath>

Hazard::Hazard(Vector2 position, Vector2 size, HazardType type)
    : Entity(position, size, EntityType::HAZARD),
      m_hazardType(type),
      m_animTimer(0.0f),
      m_startPos(position),
      m_endPos(position),
      m_moveSpeed(0.0f),
      m_movingToEnd(true),
      m_dropTriggerY(0.0f),
      m_dropResetY(0.0f),
      m_fallSpeed(0.0f),
      m_climbSpeedThreshold(0.0f),
      m_dropOnFastClimb(false),
      m_isDropping(false) {}

Hazard::Hazard(Vector2 startPos, Vector2 endPos, Vector2 size, float speed)
    : Entity(startPos, size, EntityType::HAZARD),
      m_hazardType(HazardType::THORN_CLUSTER),
      m_animTimer(0.0f),
      m_startPos(startPos),
      m_endPos(endPos),
      m_moveSpeed(speed),
      m_movingToEnd(true),
      m_dropTriggerY(0.0f),
      m_dropResetY(0.0f),
      m_fallSpeed(0.0f),
      m_climbSpeedThreshold(0.0f),
      m_dropOnFastClimb(false),
      m_isDropping(false) {}

void Hazard::SetDropTrigger(float triggerY, float resetY, float fallSpeed, float climbSpeedThreshold) {
    m_dropOnFastClimb = true;
    m_dropTriggerY = triggerY;
    m_dropResetY = resetY;
    m_fallSpeed = fallSpeed;
    m_climbSpeedThreshold = climbSpeedThreshold;
}

void Hazard::TryTriggerDrop(Vector2 playerPosition, Vector2 playerVelocity) {
    if (m_dropOnFastClimb && !m_isDropping &&
        std::abs(playerPosition.x - m_position.x) <= 320.0f &&
        playerPosition.y <= m_dropTriggerY && playerVelocity.y <= -m_climbSpeedThreshold) {
        m_isDropping = true;
    }
}

void Hazard::Update(float dt) {
    m_animTimer += dt;
    if (m_isDropping) {
        m_position.y += m_fallSpeed * dt;
        if (m_position.y >= m_dropResetY) {
            m_position = m_startPos;
            m_isDropping = false;
        }
        return;
    }
    if (m_moveSpeed <= 0.0f) return;

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

void Hazard::Draw() {
    Rectangle bounds = GetBounds();

    if (m_hazardType == HazardType::THORN_CLUSTER) {
        Texture2D& thorns = ResourceManager::GetInstance().GetTexture("thorn_cluster");
        if (thorns.id != 0) {
            Rectangle source = { 0.0f, 0.0f, static_cast<float>(thorns.width), static_cast<float>(thorns.height) };
            const float spriteWidth = 52.0f;
            const float spriteHeight = 46.0f;
            for (float x = bounds.x; x < bounds.x + bounds.width; x += spriteWidth - 8.0f) {
                Rectangle destination = { x, bounds.y + bounds.height - spriteHeight, spriteWidth, spriteHeight };
                DrawTexturePro(thorns, source, destination, { 0.0f, 0.0f }, 0.0f, WHITE);
            }
        } else {
            DrawRectangleRec(bounds, Color{ 38, 20, 28, 240 });
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
