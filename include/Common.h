#pragma once

#include "raylib.h"
#include "raymath.h"
#include <string>
#include <vector>
#include <memory>
#include <iostream>

// Entity Types for polymorphic identification
enum class EntityType {
    NONE,
    PLAYER,
    PLATFORM,
    ENEMY,
    COLLECTABLE,
    VINE,
    ANCHOR_NODE,
    BARRIER,
    HAZARD
};

// Platform Behaviors
enum class PlatformType {
    SOLID,      // Collides from all 4 directions
    ONE_WAY,    // Can jump through from bottom, stand on top
    MOVING,     // Moves between waypoints, carries entities
    TREE_BARK   // Vertical clingable tree bark
};

// Player Kinetic States for Canopy Snap
enum class PlayerMovementState {
    AIRBORNE,       // Free ballistic flight / gravity fall
    BRACHIATING,    // Hanging & swinging on a physics vine
    TAIL_PIVOTING,  // Snapped to anchor node, whipping along orbital arc
    WALL_SLIDING,   // Clinging to vertical tree bark
    POUNCING        // High-velocity kinetic smash burst
};

// Direction facing
enum class Direction {
    LEFT,
    RIGHT
};

