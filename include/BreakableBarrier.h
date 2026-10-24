#pragma once

#include "Entity.h"

enum class BarrierType {
    BRITTLE_WOOD,
    PREDATORY_FLORA
};

class BreakableBarrier : public Entity {
public:
    BreakableBarrier(Vector2 position, Vector2 size, BarrierType type = BarrierType::BRITTLE_WOOD);

    void Update(float dt) override;
    void Draw() override;
    void DrawDebug() const override;

    bool IsBroken() const { return m_isBroken; }
    void Break();

    BarrierType GetBarrierType() const { return m_barrierType; }

private:
    BarrierType m_barrierType;
    bool m_isBroken;
    float m_shakeTimer;
};
