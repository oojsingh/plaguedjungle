#pragma once

#include "Entity.h"
#include "Animation.h"

class Collectable : public Entity {
public:
    Collectable(Vector2 position, int scoreValue = 100);

    void Update(float dt) override;
    void Draw() override;
    void DrawDebug() const override;

    int GetScoreValue() const { return m_scoreValue; }
    bool IsCollected() const { return m_collected; }
    void Collect() { m_collected = true; m_active = false; }

private:
    int m_scoreValue;
    bool m_collected;
    float m_baseY;
    float m_bobTimer;

    Animation m_animSpin;
};
