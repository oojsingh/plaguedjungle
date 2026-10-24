#pragma once

#include "Common.h"

class Animation {
public:
    Animation();
    // 1D horizontal spritesheet constructor (backward compatible)
    Animation(Texture2D* texture, int frameCount, float frameSpeed, int startFrame = 0, int frameWidth = 32, int frameHeight = 32);
    // 2D grid spritesheet constructor (with row support and looping control)
    Animation(Texture2D* texture, int frameCount, float frameSpeed, int startCol, int row, int frameWidth, int frameHeight, bool loop = true);

    void Update(float dt);
    void Draw(Vector2 position, Vector2 size, bool flipX = false, Color tint = WHITE) const;
    void Reset();

    void SetFrameSpeed(float speed) { m_frameSpeed = speed; }
    void SetCurrentFrame(int frame);
    void SetLooping(bool loop) { m_loop = loop; }
    
    int GetCurrentFrame() const { return m_currentFrame; }
    int GetFrameCount() const { return m_frameCount; }
    bool IsFinished() const { return !m_loop && (m_currentFrame >= m_frameCount - 1); }

private:
    Texture2D* m_texture;
    int m_frameCount;
    int m_startCol;
    int m_row;
    int m_currentFrame;
    float m_frameSpeed; // FPS
    float m_timer;
    int m_frameWidth;
    int m_frameHeight;
    bool m_loop;
};
