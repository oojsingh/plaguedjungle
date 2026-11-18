#pragma once

#include "Entity.h"

enum class HazardType {
    THORN_CLUSTER,
    SPORE_VENT
};

class Hazard : public Entity {
public:
    Hazard(Vector2 position, Vector2 size, HazardType type = HazardType::THORN_CLUSTER);
    Hazard(Vector2 startPos, Vector2 endPos, Vector2 size, float speed);

    void Update(float dt) override;
    void Draw() override;
    void DrawDebug() const override;

    HazardType GetHazardType() const { return m_hazardType; }
    bool IsMoving() const { return m_moveSpeed > 0.0f; }
    void SetDropTrigger(float triggerY, float resetY, float fallSpeed, float climbSpeedThreshold);
    void TryTriggerDrop(Vector2 playerPosition, Vector2 playerVelocity);

private:
    HazardType m_hazardType;
    float m_animTimer;
    Vector2 m_startPos;
    Vector2 m_endPos;
    float m_moveSpeed;
    bool m_movingToEnd;
    float m_dropTriggerY;
    float m_dropResetY;
    float m_fallSpeed;
    float m_climbSpeedThreshold;
    bool m_dropOnFastClimb;
    bool m_isDropping;
};
