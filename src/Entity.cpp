#include "Entity.h"

Entity::Entity(Vector2 position, Vector2 size, EntityType type)
    : m_position(position),
      m_velocity({ 0.0f, 0.0f }),
      m_size(size),
      m_type(type),
      m_active(true),
      m_isGrounded(false),
      m_wasGrounded(false),
      m_isCeiling(false),
      m_isWallLeft(false),
      m_isWallRight(false) {}

void Entity::DrawDebug() const {
    Rectangle rect = GetBounds();
    // Bounding box outline
    DrawRectangleLinesEx(rect, 1.0f, GREEN);

    // Grounded indicator
    if (m_isGrounded) {
        DrawLineEx({ rect.x, rect.y + rect.height }, { rect.x + rect.width, rect.y + rect.height }, 2.0f, YELLOW);
    }

    // Velocity vector line
    Vector2 center = { rect.x + rect.width * 0.5f, rect.y + rect.height * 0.5f };
    DrawLineV(center, Vector2Add(center, Vector2Scale(m_velocity, 0.1f)), RED);
}
