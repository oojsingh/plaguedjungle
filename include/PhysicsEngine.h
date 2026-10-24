#pragma once

#include "Common.h"
#include "Entity.h"
#include "Platform.h"
#include <vector>

class PhysicsEngine {
public:
    PhysicsEngine(float gravity = 980.0f, float maxFallSpeed = 700.0f);

    // Apply gravity to dynamic entity
    void ApplyGravity(Entity* entity, float dt);

    // Axis-separated movement and collision resolution against platforms
    void UpdateEntityPhysics(Entity* entity, const std::vector<Platform*>& platforms, float dt);

    // AABB intersection utility
    static bool CheckAABB(const Rectangle& a, const Rectangle& b);

    // Physics parameters tuning
    float GetGravity() const { return m_gravity; }
    void SetGravity(float g) { m_gravity = g; }

    float GetMaxFallSpeed() const { return m_maxFallSpeed; }
    void SetMaxFallSpeed(float speed) { m_maxFallSpeed = speed; }

private:
    float m_gravity;
    float m_maxFallSpeed;

    void ResolveHorizontal(Entity* entity, const std::vector<Platform*>& platforms, float dt);
    void ResolveVertical(Entity* entity, const std::vector<Platform*>& platforms, float dt);
};
