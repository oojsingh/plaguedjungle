#pragma once

#include "Entity.h"
#include "Animation.h"
#include <vector>

class Vine;
class AnchorNode;

class Player : public Entity {
public:
    Player(Vector2 position);

    void HandleInput(float dt, const std::vector<Vine*>& vines, const std::vector<AnchorNode*>& anchorNodes);
    void Update(float dt) override;
    void Draw() override;
    void DrawDebug() const override;

    // State & Momentum Getters
    PlayerMovementState GetMovementState() const { return m_state; }
    float GetPounceMeter() const { return m_pounceMeter; }
    float GetSpeed() const;
    bool IsPouncing() const { return m_state == PlayerMovementState::POUNCING; }
    bool IsFacingRight() const { return m_facingRight; }
    const AnchorNode* GetTargetAnchorNode() const { return m_targetAnchorNode; }

    // Resets
    void Reset(Vector2 spawnPos);

    // Actions
    bool TryGrabVine(const std::vector<Vine*>& vines);
    void ReleaseVine();

    bool TryTailSnap(const std::vector<AnchorNode*>& anchorNodes);
    void ReleaseTailSnap();

    bool TryPounce();

private:
    PlayerMovementState m_state;

    // Movement attributes
    float m_moveSpeed;
    float m_acceleration;
    float m_deceleration;
    float m_airControl;
    float m_jumpForce;

    // Brachiation (Vines)
    Vine* m_currentVine;

    // Tail Pivot (Anchor Nodes)
    AnchorNode* m_tailNode;
    AnchorNode* m_targetAnchorNode;
    float m_tailAngle;
    float m_tailAngularVelocity;
    float m_tailRadius;
    float m_tailTimer;
    float m_tailMaxDuration;

    // Wall Pounce & Kinetic Meter
    float m_pounceMeter;       // 0.0 to 100.0
    float m_pounceTimer;       // Active duration of pounce smash
    Vector2 m_pounceDirection;
    float m_wallClingTimer;

    // Jump / Fluidity
    float m_coyoteTime;
    float m_coyoteTimer;
    float m_jumpBufferTime;
    float m_jumpBufferTimer;

    // Visuals & Animations
    bool m_facingRight;
    float m_sweetSpotFeedbackTimer;
    Vector2 m_tailTipPosition;
    float m_tailWhipAnim;

    // Spritesheet Animations from assets
    Animation m_animIdle;
    Animation m_animRun;
    Animation m_animJump;
    Animation m_animSwing;
    Animation m_animTailHook;
    Animation m_animWallCling;
    Animation* m_currentAnim;

    void UpdateAnimations(float dt);
    void UpdateTailPhysics(float dt);
};
