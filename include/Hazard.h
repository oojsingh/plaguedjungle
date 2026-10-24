#pragma once

#include "Entity.h"

enum class HazardType {
    THORN_CLUSTER,
    SPORE_VENT
};

class Hazard : public Entity {
public:
    Hazard(Vector2 position, Vector2 size, HazardType type = HazardType::THORN_CLUSTER);

    void Update(float dt) override;
    void Draw() override;
    void DrawDebug() const override;

    HazardType GetHazardType() const { return m_hazardType; }

private:
    HazardType m_hazardType;
    float m_animTimer;
};
