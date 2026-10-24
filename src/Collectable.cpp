#include "Collectable.h"
#include "ResourceManager.h"
#include <cmath>

Collectable::Collectable(Vector2 position, int scoreValue)
    : Entity(position, { 18.0f, 18.0f }, EntityType::COLLECTABLE),
      m_scoreValue(scoreValue),
      m_collected(false),
      m_baseY(position.y),
      m_bobTimer(0.0f) {

    Texture2D& itemsTex = ResourceManager::GetInstance().GetTexture("items");
    // Frames 0 & 1 in items.png (each 32x32)
    m_animSpin = Animation(&itemsTex, 2, 3.0f, 0, 32, 32);
}

void Collectable::Update(float dt) {
    if (!m_active) return;

    m_bobTimer += dt * 3.0f;
    // Gentle sine wave floating effect
    m_position.y = m_baseY + std::sin(m_bobTimer) * 4.0f;

    m_animSpin.Update(dt);
}

void Collectable::Draw() {
    if (!m_active) return;
    Rectangle bounds = GetBounds();
    DrawRectangleRec(bounds, Color{ 241, 196, 15, 255 }); // Gold box
    DrawRectangleLinesEx(bounds, 1.5f, Color{ 180, 140, 10, 255 });
}

void Collectable::DrawDebug() const {
    if (!m_active) return;
    Entity::DrawDebug();
}
