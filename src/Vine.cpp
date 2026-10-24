#include "Vine.h"
#include "ParticleSystem.h"
#include <cmath>
#include <algorithm>

Vine::Vine(Vector2 anchorPos, float length)
    : Entity(anchorPos, { 24.0f, length }, EntityType::VINE),
      m_anchorPos(anchorPos),
      m_length(length),
      m_angle(0.0f),
      m_angularVelocity(0.0f),
      m_isAttached(false),
      m_idleTimer(0.0f) {}

Vector2 Vine::GetTipPosition() const {
    return {
        m_anchorPos.x + std::sin(m_angle) * m_length,
        m_anchorPos.y + std::cos(m_angle) * m_length
    };
}

bool Vine::CanGrab(Vector2 pos, float reachRadius) const {
    Vector2 tip = GetTipPosition();
    float distSq = (pos.x - tip.x) * (pos.x - tip.x) + (pos.y - tip.y) * (pos.y - tip.y);
    return distSq <= (reachRadius * reachRadius);
}

void Vine::AttachPlayer(Vector2 playerVel) {
    m_isAttached = true;

    // Convert linear player velocity into initial angular velocity:
    // v_tangent = v.x * cos(angle) - v.y * sin(angle)
    float vTangent = playerVel.x * std::cos(m_angle) - playerVel.y * std::sin(m_angle);
    m_angularVelocity = vTangent / m_length;

    // Cap initial rotational velocity to prevent astronomical spins
    m_angularVelocity = std::clamp(m_angularVelocity, -9.0f, 9.0f);
}

void Vine::DetachPlayer() {
    m_isAttached = false;
}

void Vine::ApplyPumpTorque(float direction, float dt) {
    if (!m_isAttached || direction == 0.0f) return;

    // Add angular acceleration in the player's pushed direction
    // Pumping adds 9.0 rad/s^2 when working with momentum
    float pumpAccel = 9.5f;

    // Reward player for pumping in the direction they are swinging
    if ((direction > 0.0f && m_angularVelocity >= -0.5f) ||
        (direction < 0.0f && m_angularVelocity <= 0.5f)) {
        m_angularVelocity += direction * pumpAccel * dt;
    } else {
        // Counter-pump applies brake
        m_angularVelocity += direction * (pumpAccel * 0.5f) * dt;
    }

    // Limit maximum angular speed to keep swing controllable and brutal
    m_angularVelocity = std::clamp(m_angularVelocity, -8.5f, 8.5f);
}

Vector2 Vine::CalculateReleaseVelocity(bool* outSweetSpot) const {
    // Tangential direction: (-cos(angle), sin(angle)) for forward/backward
    float linearSpeed = m_angularVelocity * m_length;

    Vector2 launchVel = {
        std::cos(m_angle) * linearSpeed,
        -std::sin(m_angle) * linearSpeed
    };

    // Sweet spot check:
    // Releasing while swinging upward (launchVel.y < -80.0f) at high speed
    bool isSweet = false;
    if (std::abs(linearSpeed) > 320.0f && launchVel.y < -50.0f) {
        isSweet = true;
        // Apply 25% momentum launch multiplier
        launchVel.x *= 1.30f;
        launchVel.y *= 1.35f;
    }

    if (outSweetSpot) {
        *outSweetSpot = isSweet;
    }

    return launchVel;
}

void Vine::Update(float dt) {
    const float gravity = 1100.0f; // Gravity torque

    if (m_isAttached) {
        // Pendulum differential equation: alpha = -(g/L) * sin(theta)
        float angularAccel = -(gravity / m_length) * std::sin(m_angle);
        m_angularVelocity += angularAccel * dt;

        // Subtle air resistance during swing
        m_angularVelocity *= (1.0f - dt * 0.15f);

        m_angle += m_angularVelocity * dt;

        // Emit kinetic trail at the tip if swinging fast
        if (std::abs(m_angularVelocity * m_length) > 350.0f) {
            Vector2 tip = GetTipPosition();
            ParticleSystem::GetInstance().EmitTrail(
                tip,
                { std::cos(m_angle) * m_angularVelocity * 0.5f, -std::sin(m_angle) * m_angularVelocity * 0.5f },
                Color{ 46, 204, 113, 200 },
                4.0f,
                0.2f
            );
        }
    } else {
        // Idle gentle sway or settling physics
        if (std::abs(m_angularVelocity) > 0.1f || std::abs(m_angle) > 0.05f) {
            float angularAccel = -(gravity / m_length) * std::sin(m_angle);
            m_angularVelocity += angularAccel * dt;
            m_angularVelocity *= (1.0f - dt * 1.8f); // Damp quickly back to rest
            m_angle += m_angularVelocity * dt;
        } else {
            // Ambient breeze sway
            m_idleTimer += dt;
            m_angle = std::sin(m_idleTimer * 1.8f) * 0.08f;
            m_angularVelocity = 0.0f;
        }
    }

    // Keep angle within [-1.55, 1.55] rad (~88 degrees) to prevent looping upside down
    if (m_angle > 1.55f) {
        m_angle = 1.55f;
        m_angularVelocity = -std::abs(m_angularVelocity) * 0.5f;
    } else if (m_angle < -1.55f) {
        m_angle = -1.55f;
        m_angularVelocity = std::abs(m_angularVelocity) * 0.5f;
    }

    // Update position/bounding box
    Vector2 tip = GetTipPosition();
    m_position = { std::min(m_anchorPos.x, tip.x) - 10.0f, m_anchorPos.y };
    m_size = { std::abs(tip.x - m_anchorPos.x) + 20.0f, m_length + 20.0f };
}

void Vine::Draw() {
    Vector2 tip = GetTipPosition();

    // 1. Draw top anchor knot to branch
    DrawCircleV(m_anchorPos, 6.0f, Color{ 40, 25, 12, 255 });
    DrawCircleV(m_anchorPos, 4.0f, Color{ 85, 55, 25, 255 });

    // 2. Draw vine segments (multi-segment curve for natural braided rope look)
    int segments = 12;
    Vector2 prevPt = m_anchorPos;

    for (int i = 1; i <= segments; ++i) {
        float t = static_cast<float>(i) / segments;
        float currentAngle = m_angle * t;
        float currentLen = m_length * t;

        Vector2 pt = {
            m_anchorPos.x + std::sin(currentAngle) * currentLen,
            m_anchorPos.y + std::cos(currentAngle) * currentLen
        };

        // Thick vine body
        DrawLineEx(prevPt, pt, 5.0f, Color{ 34, 110, 48, 255 });
        DrawLineEx(prevPt, pt, 2.5f, Color{ 68, 175, 78, 255 });

        // Decorative leaves along vine
        if (i % 3 == 0) {
            float leafDir = (i % 2 == 0) ? 1.0f : -1.0f;
            Vector2 leafPt = { pt.x + leafDir * 7.0f, pt.y - 2.0f };
            DrawCircleV(leafPt, 3.5f, Color{ 118, 215, 90, 230 });
        }

        prevPt = pt;
    }

    // 3. Draw tip grab knot / glowing moss handle
    Color tipGlow = m_isAttached ? Color{ 0, 255, 200, 255 } : Color{ 120, 240, 90, 255 };
    DrawCircleV(tip, 8.0f, Color{ 25, 80, 35, 255 });
    DrawCircleV(tip, 5.0f, tipGlow);

    // If swinging fast on upswing, show sweet-spot launch ring cue
    if (m_isAttached && std::abs(m_angularVelocity * m_length) > 320.0f) {
        float linearSpeed = m_angularVelocity * m_length;
        if (-std::sin(m_angle) * linearSpeed < -50.0f) {
            DrawCircleLines(static_cast<int>(tip.x), static_cast<int>(tip.y), 18.0f, Color{ 255, 215, 0, 200 });
            DrawCircleLines(static_cast<int>(tip.x), static_cast<int>(tip.y), 24.0f, Color{ 255, 255, 100, 140 });
        }
    }
}

void Vine::DrawDebug() const {
    Vector2 tip = GetTipPosition();
    DrawCircleLines(static_cast<int>(tip.x), static_cast<int>(tip.y), 36.0f, RED);
    DrawText(TextFormat("Omega: %.2f", m_angularVelocity), static_cast<int>(tip.x + 10), static_cast<int>(tip.y), 10, YELLOW);
}
