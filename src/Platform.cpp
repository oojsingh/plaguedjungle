#include "Platform.h"
#include "ResourceManager.h"

Platform::Platform(Vector2 position, Vector2 size, PlatformType type)
    : Entity(position, size, EntityType::PLATFORM),
      m_platformType(type),
      m_startPos(position),
      m_endPos(position),
      m_speed(0.0f),
      m_movingToEnd(true),
      m_platformVelocity({ 0.0f, 0.0f }) {}

Platform::Platform(Vector2 startPos, Vector2 endPos, Vector2 size, float speed)
    : Entity(startPos, size, EntityType::PLATFORM),
      m_platformType(PlatformType::MOVING),
      m_startPos(startPos),
      m_endPos(endPos),
      m_speed(speed),
      m_movingToEnd(true),
      m_platformVelocity({ 0.0f, 0.0f }) {}

void Platform::Update(float dt) {
    if (m_platformType != PlatformType::MOVING) return;

    Vector2 target = m_movingToEnd ? m_endPos : m_startPos;
    Vector2 dir = Vector2Subtract(target, m_position);
    float dist = Vector2Length(dir);

    float moveStep = m_speed * dt;
    if (dist <= moveStep) {
        m_position = target;
        m_movingToEnd = !m_movingToEnd;
        // Turn around direction
        Vector2 nextTarget = m_movingToEnd ? m_endPos : m_startPos;
        Vector2 nextDir = Vector2Normalize(Vector2Subtract(nextTarget, m_position));
        m_platformVelocity = Vector2Scale(nextDir, m_speed);
    } else {
        Vector2 normDir = Vector2Normalize(dir);
        m_platformVelocity = Vector2Scale(normDir, m_speed);
        m_position = Vector2Add(m_position, Vector2Scale(normDir, moveStep));
    }
}

void Platform::Draw() {
    Rectangle bounds = GetBounds();

    if (m_platformType == PlatformType::ONE_WAY) {
        // One-Way Jump-Through Platform: Warm amber
        DrawRectangleRec(bounds, Color{ 230, 126, 34, 210 });
        DrawRectangleLinesEx(bounds, 1.5f, Color{ 180, 85, 15, 255 });
        // Solid accent on the landing surface
        DrawLineEx({ bounds.x, bounds.y }, { bounds.x + bounds.width, bounds.y }, 3.0f, Color{ 255, 215, 0, 255 });
    } else if (m_platformType == PlatformType::MOVING) {
        // Moving Platform: Cyan/Teal
        DrawRectangleRec(bounds, Color{ 26, 188, 156, 240 });
        DrawRectangleLinesEx(bounds, 2.0f, Color{ 18, 130, 110, 255 });
        // Subtle center grip indicator
        float cx = bounds.x + bounds.width * 0.5f;
        float cy = bounds.y + bounds.height * 0.5f;
        DrawCircle(static_cast<int>(cx), static_cast<int>(cy), 3.5f, WHITE);
    } else if (m_platformType == PlatformType::TREE_BARK) {
        // Ancient Giant Redwood Trunk / Clingable Vertical Bark
        DrawRectangleRec(bounds, Color{ 88, 50, 24, 255 });
        DrawRectangleLinesEx(bounds, 2.0f, Color{ 60, 32, 14, 255 });

        // Vertical bark grooves & moss patches
        for (float x = bounds.x + 8.0f; x < bounds.x + bounds.width - 4.0f; x += 14.0f) {
            DrawLineEx({ x, bounds.y }, { x, bounds.y + bounds.height }, 1.5f, Color{ 55, 28, 12, 230 });
        }
        // Moss growth patches on the bark
        for (float y = bounds.y + 20.0f; y < bounds.y + bounds.height - 10.0f; y += 45.0f) {
            DrawRectangle(static_cast<int>(bounds.x + 2), static_cast<int>(y), 8, 14, Color{ 46, 125, 50, 220 });
            DrawRectangle(static_cast<int>(bounds.x + bounds.width - 10), static_cast<int>(y + 15), 8, 12, Color{ 46, 125, 50, 220 });
        }
    } else {
        // Standard Solid Ancient Canopy Branch
        DrawRectangleRec(bounds, Color{ 60, 42, 28, 255 });
        DrawRectangleLinesEx(bounds, 2.0f, Color{ 42, 28, 18, 255 });
        // Lush canopy moss on the landing surface
        DrawLineEx({ bounds.x, bounds.y }, { bounds.x + bounds.width, bounds.y }, 3.5f, Color{ 46, 204, 113, 255 });
    }
}

void Platform::DrawDebug() const {
    Entity::DrawDebug();

    // Moving path line in debug mode
    if (m_platformType == PlatformType::MOVING) {
        DrawLineV(
            { m_startPos.x + m_size.x * 0.5f, m_startPos.y + m_size.y * 0.5f },
            { m_endPos.x + m_size.x * 0.5f, m_endPos.y + m_size.y * 0.5f },
            Fade(YELLOW, 0.6f)
        );
    }
}
