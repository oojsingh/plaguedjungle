#pragma once

#include "Entity.h"

enum class AnchorType {
    GLOWING_MUSHROOM,
    STURDY_BRANCH_KNOT
};

class AnchorNode : public Entity {
public:
    AnchorNode(Vector2 position, AnchorType type = AnchorType::GLOWING_MUSHROOM, float radius = 150.0f);

    void Update(float dt) override;
    void Draw() override;
    void DrawDebug() const override;

    bool IsInRange(Vector2 playerCenter) const;
    Vector2 GetCenter() const { return m_position; }
    float GetRadius() const { return m_radius; }
    AnchorType GetAnchorType() const { return m_anchorType; }

    void SetHighlighted(bool highlighted) { m_isHighlighted = highlighted; }
    bool IsHighlighted() const { return m_isHighlighted; }

    void TriggerSnap();
    bool CanSnap() const { return m_cooldownTimer <= 0.0f; }

private:
    AnchorType m_anchorType;
    float m_radius;
    bool m_isHighlighted;
    float m_cooldownTimer;
    float m_pulseTimer;
};
