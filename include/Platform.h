#pragma once

#include "Entity.h"

class Platform : public Entity {
public:
    // Constructor for static platforms (SOLID or ONE_WAY)
    Platform(Vector2 position, Vector2 size, PlatformType type = PlatformType::SOLID);

    // Constructor for moving platforms
    Platform(Vector2 startPos, Vector2 endPos, Vector2 size, float speed);

    void Update(float dt) override;
    void Draw() override;
    void DrawDebug() const override;

    PlatformType GetPlatformType() const { return m_platformType; }
    Vector2 GetPlatformVelocity() const { return m_platformVelocity; }

private:
    PlatformType m_platformType;

    // Moving platform mechanics
    Vector2 m_startPos;
    Vector2 m_endPos;
    float m_speed;
    bool m_movingToEnd;
    Vector2 m_platformVelocity;
};
