#include "PhysicsEngine.h"
#include <algorithm>

PhysicsEngine::PhysicsEngine(float gravity, float maxFallSpeed)
    : m_gravity(gravity), m_maxFallSpeed(maxFallSpeed) {}

bool PhysicsEngine::CheckAABB(const Rectangle& a, const Rectangle& b) {
    return CheckCollisionRecs(a, b);
}

void PhysicsEngine::ApplyGravity(Entity* entity, float dt) {
    if (!entity) return;

    Vector2 vel = entity->GetVelocity();
    vel.y += m_gravity * dt;
    if (vel.y > m_maxFallSpeed) {
        vel.y = m_maxFallSpeed;
    }
    entity->SetVelocity(vel);
}

void PhysicsEngine::UpdateEntityPhysics(Entity* entity, const std::vector<Platform*>& platforms, float dt) {
    if (!entity || !entity->IsActive()) return;

    // Reset temporary collision state flags before new step
    entity->ResetCollisionFlags();

    // 1. First resolve X axis movement & collisions
    ResolveHorizontal(entity, platforms, dt);

    // 2. Then resolve Y axis movement & collisions
    ResolveVertical(entity, platforms, dt);
}

void PhysicsEngine::ResolveHorizontal(Entity* entity, const std::vector<Platform*>& platforms, float dt) {
    Vector2 pos = entity->GetPosition();
    Vector2 vel = entity->GetVelocity();
    Vector2 size = entity->GetSize();

    float targetX = pos.x + vel.x * dt;
    Rectangle nextXRect = { targetX, pos.y, size.x, size.y };

    for (const auto* plat : platforms) {
        if (!plat || !plat->IsActive()) continue;

        // One-way platforms do not block horizontal movement
        if (plat->GetPlatformType() == PlatformType::ONE_WAY) continue;

        Rectangle platRect = plat->GetBounds();

        if (CheckAABB(nextXRect, platRect)) {
            if (vel.x > 0.0f) {
                // Moving right, hit left edge of platform
                targetX = platRect.x - size.x;
                vel.x = 0.0f;
                entity->SetWallRight(true);
            } else if (vel.x < 0.0f) {
                // Moving left, hit right edge of platform
                targetX = platRect.x + platRect.width;
                vel.x = 0.0f;
                entity->SetWallLeft(true);
            }
            nextXRect.x = targetX;
        }
    }

    pos.x = targetX;
    entity->SetPosition(pos);
    entity->SetVelocity(vel);
}

void PhysicsEngine::ResolveVertical(Entity* entity, const std::vector<Platform*>& platforms, float dt) {
    Vector2 pos = entity->GetPosition();
    Vector2 vel = entity->GetVelocity();
    Vector2 size = entity->GetSize();

    float targetY = pos.y + vel.y * dt;
    Rectangle nextYRect = { pos.x, targetY, size.x, size.y };

    for (const auto* plat : platforms) {
        if (!plat || !plat->IsActive()) continue;

        Rectangle platRect = plat->GetBounds();

        if (plat->GetPlatformType() == PlatformType::ONE_WAY) {
            // One-way platform logic:
            // Only collide if moving downwards, feet were above platform before this frame,
            // and feet are now at or past the platform top edge.
            float previousFeetY = pos.y + size.y;
            float newFeetY = targetY + size.y;

            if (vel.y >= 0.0f && previousFeetY <= platRect.y + 4.0f && newFeetY >= platRect.y) {
                // Horizontal overlap check
                if (pos.x + size.x > platRect.x && pos.x < platRect.x + platRect.width) {
                    targetY = platRect.y - size.y;
                    vel.y = 0.0f;
                    entity->SetGrounded(true);
                    nextYRect.y = targetY;
                }
            }
        } else {
            // Solid or Moving platform
            if (CheckAABB(nextYRect, platRect)) {
                if (vel.y > 0.0f) {
                    // Falling onto platform from above
                    targetY = platRect.y - size.y;
                    vel.y = 0.0f;
                    entity->SetGrounded(true);

                    // If standing on a moving platform, carry the entity
                    if (plat->GetPlatformType() == PlatformType::MOVING) {
                        Vector2 platVel = plat->GetPlatformVelocity();
                        pos.x += platVel.x * dt;
                    }
                } else if (vel.y < 0.0f) {
                    // Jumping into ceiling from below
                    targetY = platRect.y + platRect.height;
                    vel.y = 0.0f;
                    entity->SetCeilingHit(true);
                }
                nextYRect.y = targetY;
            }
        }
    }

    pos.y = targetY;
    entity->SetPosition(pos);
    entity->SetVelocity(vel);
}
