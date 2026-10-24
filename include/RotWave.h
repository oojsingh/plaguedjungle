#pragma once

#include "Common.h"

class RotWave {
public:
    RotWave(float initialGroundY = 520.0f, float riseSpeed = 12.0f);

    void Reset(float groundY = 520.0f);
    void Update(float dt, Vector2 playerPos);
    void Draw(const Camera2D& camera) const;

    bool IsSubmerged(Vector2 playerPos, Vector2 playerSize) const;

    float GetCurrentY() const { return m_currentY; }
    float GetCurrentX() const { return m_currentX; }
    void SetCurrentY(float y) { m_currentY = y; }

private:
    float m_currentY;
    float m_startY;
    float m_riseSpeed;
    float m_currentX;
    float m_creepSpeedX;
    float m_waveTimer;
};
