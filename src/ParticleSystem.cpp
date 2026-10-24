#include "ParticleSystem.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>

ParticleSystem& ParticleSystem::GetInstance() {
    static ParticleSystem instance;
    return instance;
}

ParticleSystem::ParticleSystem() {
    m_particles.reserve(1000);
}

void ParticleSystem::Clear() {
    m_particles.clear();
}

void ParticleSystem::Update(float dt) {
    for (auto& p : m_particles) {
        if (!p.active) continue;

        p.life -= dt;
        if (p.life <= 0.0f) {
            p.active = false;
            continue;
        }

        p.position.x += p.velocity.x * dt;
        p.position.y += p.velocity.y * dt;

        // Mild deceleration/friction
        p.velocity.x *= (1.0f - dt * 2.0f);
        p.velocity.y *= (1.0f - dt * 2.0f);
    }

    // Clean up inactive particles periodically
    m_particles.erase(
        std::remove_if(m_particles.begin(), m_particles.end(), [](const Particle& p) { return !p.active; }),
        m_particles.end()
    );
}

void ParticleSystem::Draw() const {
    for (const auto& p : m_particles) {
        if (!p.active) continue;

        float alphaRatio = std::clamp(p.life / p.maxLife, 0.0f, 1.0f);
        Color c = p.color;
        c.a = static_cast<unsigned char>(c.a * alphaRatio);

        float currentSize = p.size * alphaRatio;
        if (currentSize > 0.5f) {
            DrawCircleV(p.position, currentSize, c);
        }
    }
}

void ParticleSystem::EmitTrail(Vector2 pos, Vector2 vel, Color color, float size, float life) {
    // Add jitter
    float jx = ((rand() % 100) / 100.0f - 0.5f) * 6.0f;
    float jy = ((rand() % 100) / 100.0f - 0.5f) * 6.0f;
    Vector2 pVel = { -vel.x * 0.15f + jx, -vel.y * 0.15f + jy };

    m_particles.push_back({
        { pos.x + jx, pos.y + jy },
        pVel,
        color,
        size,
        life,
        life,
        true
    });
}

void ParticleSystem::EmitBurst(Vector2 pos, int count, Color color, float minSpeed, float maxSpeed, float life) {
    for (int i = 0; i < count; ++i) {
        float angle = ((rand() % 360) * DEG2RAD);
        float speed = minSpeed + (rand() % 1000) / 1000.0f * (maxSpeed - minSpeed);
        Vector2 vel = { std::cos(angle) * speed, std::sin(angle) * speed };
        float pLife = life * (0.6f + (rand() % 100) / 100.0f * 0.8f);
        float size = 2.0f + (rand() % 100) / 100.0f * 3.5f;

        m_particles.push_back({
            pos,
            vel,
            color,
            size,
            pLife,
            pLife,
            true
        });
    }
}

void ParticleSystem::EmitWoodSplinters(Vector2 pos, int count) {
    Color woodDark = { 120, 60, 20, 255 };
    Color woodLight = { 185, 120, 50, 255 };
    Color mossGreen = { 76, 175, 80, 255 };

    for (int i = 0; i < count; ++i) {
        float angle = ((rand() % 360) * DEG2RAD);
        float speed = 150.0f + (rand() % 400);
        Vector2 vel = { std::cos(angle) * speed, std::sin(angle) * speed };
        Color c = (i % 3 == 0) ? mossGreen : ((i % 2 == 0) ? woodDark : woodLight);

        m_particles.push_back({
            pos,
            vel,
            c,
            3.0f + (rand() % 4),
            0.5f + (rand() % 50) / 100.0f,
            0.6f,
            true
        });
    }
}

void ParticleSystem::EmitTailSnap(Vector2 pos, Vector2 targetNode) {
    // Sparks along the snap vector
    Vector2 diff = Vector2Subtract(targetNode, pos);
    float dist = Vector2Length(diff);
    Vector2 dir = (dist > 0.001f) ? Vector2Scale(diff, 1.0f / dist) : Vector2{ 1.0f, 0.0f };

    int segments = std::min(15, static_cast<int>(dist / 10.0f));
    for (int i = 0; i <= segments; ++i) {
        float t = static_cast<float>(i) / segments;
        Vector2 p = Vector2Add(pos, Vector2Scale(diff, t));
        float jitterX = ((rand() % 100) / 100.0f - 0.5f) * 12.0f;
        float jitterY = ((rand() % 100) / 100.0f - 0.5f) * 12.0f;

        m_particles.push_back({
            { p.x + jitterX, p.y + jitterY },
            { dir.x * 50.0f + jitterX * 4.0f, dir.y * 50.0f + jitterY * 4.0f },
            Color{ 0, 255, 220, 255 }, // Cyan kinetic shock
            3.5f,
            0.22f,
            0.22f,
            true
        });
    }

    // Radial flash at the node
    EmitBurst(targetNode, 18, Color{ 0, 255, 230, 255 }, 120.0f, 320.0f, 0.35f);
}

void ParticleSystem::EmitSporeFog(Vector2 pos, float width) {
    if (rand() % 3 != 0) return; // limit emission rate
    float px = pos.x + (rand() % static_cast<int>(std::max(10.0f, width)));
    float py = pos.y + ((rand() % 40) - 20.0f);
    float vy = -30.0f - (rand() % 60);
    float vx = ((rand() % 100) / 100.0f - 0.5f) * 35.0f;

    Color sporeColor = (rand() % 2 == 0) ? Color{ 160, 32, 240, 180 } : Color{ 50, 205, 50, 180 };
    m_particles.push_back({
        { px, py },
        { vx, vy },
        sporeColor,
        3.0f + (rand() % 4),
        0.8f + (rand() % 50) / 100.0f,
        1.2f,
        true
    });
}
