#pragma once

#include "Entity.h"
#include "Animation.h"

class Enemy : public Entity {
public:
    Enemy(Vector2 position, float patrolDistance = 120.0f, float speed = 50.0f);

    void Update(float dt) override;
    void Draw() override;
    void DrawDebug() const override;

    float GetSpeed() const { return m_speed; }
    void SetSpeed(float speed) { m_speed = speed; }

private:
    float m_startX;
    float m_patrolDistance;
    float m_speed;
    bool m_movingRight;

    Animation m_animWalk;
};
