#pragma once

#include "Common.h"
#include <vector>

struct Particle {
    Vector2 position;
    Vector2 velocity;
    Color color;
    float size;
    float life;
    float maxLife;
    bool active;
};

class ParticleSystem {
public:
    static ParticleSystem& GetInstance();

    void Update(float dt);
    void Draw() const;
    void Clear();

    // Spawners for game feel
    void EmitTrail(Vector2 pos, Vector2 vel, Color color, float size = 4.0f, float life = 0.25f);
    void EmitBurst(Vector2 pos, int count, Color color, float minSpeed = 80.0f, float maxSpeed = 300.0f, float life = 0.4f);
    void EmitWoodSplinters(Vector2 pos, int count);
    void EmitTailSnap(Vector2 pos, Vector2 targetNode);
    void EmitSporeFog(Vector2 pos, float width);

private:
    ParticleSystem();
    std::vector<Particle> m_particles;
};
