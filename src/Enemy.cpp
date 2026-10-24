#include "Enemy.h"
#include "ResourceManager.h"
#include <cmath>

Enemy::Enemy(Vector2 position, float patrolDistance, float speed)
    : Entity(position, { 28.0f, 22.0f }, EntityType::ENEMY),
      m_startX(position.x),
      m_patrolDistance(patrolDistance),
      m_speed(speed),
      m_movingRight(true) {

    Texture2D& itemsTex = ResourceManager::GetInstance().GetTexture("items");
    // Frames 2 & 3 in items.png (each 32x32)
    m_animWalk = Animation(&itemsTex, 2, 4.0f, 2, 32, 32);
}

void Enemy::Update(float dt) {
    // Basic patrol kinematics
    if (m_movingRight) {
        m_velocity.x = m_speed;
        if (m_position.x >= m_startX + m_patrolDistance || m_isWallRight) {
            m_movingRight = false;
        }
    } else {
        m_velocity.x = -m_speed;
        if (m_position.x <= m_startX || m_isWallLeft) {
            m_movingRight = true;
        }
    }

    m_animWalk.Update(dt);
}

void Enemy::Draw() {
    Rectangle bounds = GetBounds();
    DrawRectangleRec(bounds, Color{ 231, 76, 60, 255 }); // Coral red enemy box
    DrawRectangleLinesEx(bounds, 2.0f, Color{ 150, 40, 30, 255 });

    // Direction indicator
    float eyeX = m_movingRight ? (bounds.x + bounds.width - 6.0f) : (bounds.x + 2.0f);
    DrawRectangle(static_cast<int>(eyeX), static_cast<int>(bounds.y + 4.0f), 4, 4, YELLOW);
}

void Enemy::DrawDebug() const {
    Entity::DrawDebug();
    // Draw patrol range line
    DrawLine(static_cast<int>(m_startX), static_cast<int>(m_position.y + m_size.y * 0.5f),
              static_cast<int>(m_startX + m_patrolDistance), static_cast<int>(m_position.y + m_size.y * 0.5f),
              PURPLE);
}
