#pragma once

#include "Entity.h"

class Vine : public Entity {
public:
    Vine(Vector2 anchorPos, float length);

    void Update(float dt) override;
    void Draw() override;
    void DrawDebug() const override;

    // Grab zone and attachment
    bool CanGrab(Vector2 pos, float reachRadius = 36.0f) const;
    Vector2 GetTipPosition() const;
    Vector2 GetAnchorPosition() const { return m_anchorPos; }

    float GetLength() const { return m_length; }
    float GetAngle() const { return m_angle; }
    float GetAngularVelocity() const { return m_angularVelocity; }
    void SetAngularVelocity(float omega) { m_angularVelocity = omega; }

    // Momentum transfer and swing pumping
    void AttachPlayer(Vector2 playerVel);
    void DetachPlayer();
    void ApplyPumpTorque(float direction, float dt); // -1.0 for left, +1.0 for right
    Vector2 CalculateReleaseVelocity(bool* outSweetSpot = nullptr) const;

    bool IsPlayerAttached() const { return m_isAttached; }

private:
    Vector2 m_anchorPos;
    float m_length;
    float m_angle;             // 0 = straight down, -pi/2 = left, +pi/2 = right
    float m_angularVelocity;   // rad/s
    bool m_isAttached;

    float m_idleTimer;
};
