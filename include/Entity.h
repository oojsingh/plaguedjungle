#pragma once

#include "Common.h"

class Entity {
public:
    Entity(Vector2 position, Vector2 size, EntityType type);
    virtual ~Entity() = default;

    // Polymorphic lifecycle methods
    virtual void Update(float dt) = 0;
    virtual void Draw() = 0;
    virtual void DrawDebug() const;

    // Spatial & Bounding Box
    Rectangle GetBounds() const {
        return { m_position.x, m_position.y, m_size.x, m_size.y };
    }

    // Encapsulation: Getters & Setters
    Vector2 GetPosition() const { return m_position; }
    void SetPosition(Vector2 pos) { m_position = pos; }
    void SetPositionX(float x) { m_position.x = x; }
    void SetPositionY(float y) { m_position.y = y; }

    Vector2 GetVelocity() const { return m_velocity; }
    void SetVelocity(Vector2 vel) { m_velocity = vel; }
    void SetVelocityX(float vx) { m_velocity.x = vx; }
    void SetVelocityY(float vy) { m_velocity.y = vy; }

    Vector2 GetSize() const { return m_size; }
    void SetSize(Vector2 size) { m_size = size; }

    EntityType GetType() const { return m_type; }

    bool IsActive() const { return m_active; }
    void Destroy() { m_active = false; }

    // Collision State
    bool IsGrounded() const { return m_isGrounded; }
    void SetGrounded(bool grounded) { m_isGrounded = grounded; }

    bool WasGrounded() const { return m_wasGrounded; }
    void SetWasGrounded(bool wasGrounded) { m_wasGrounded = wasGrounded; }

    bool IsCeilingHit() const { return m_isCeiling; }
    void SetCeilingHit(bool ceiling) { m_isCeiling = ceiling; }

    bool IsWallLeft() const { return m_isWallLeft; }
    void SetWallLeft(bool wall) { m_isWallLeft = wall; }

    bool IsWallRight() const { return m_isWallRight; }
    void SetWallRight(bool wall) { m_isWallRight = wall; }

    void ResetCollisionFlags() {
        m_wasGrounded = m_isGrounded;
        m_isGrounded = false;
        m_isCeiling = false;
        m_isWallLeft = false;
        m_isWallRight = false;
    }

protected:
    Vector2 m_position;
    Vector2 m_velocity;
    Vector2 m_size;
    EntityType m_type;
    bool m_active;

    // Physics collision tracking
    bool m_isGrounded;
    bool m_wasGrounded;
    bool m_isCeiling;
    bool m_isWallLeft;
    bool m_isWallRight;
};
