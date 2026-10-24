#include "Animation.h"
#include <algorithm>

Animation::Animation()
    : m_texture(nullptr), m_frameCount(1), m_startCol(0), m_row(0), m_currentFrame(0),
      m_frameSpeed(1.0f), m_timer(0.0f), m_frameWidth(32), m_frameHeight(32), m_loop(true) {}

Animation::Animation(Texture2D* texture, int frameCount, float frameSpeed, int startFrame, int frameWidth, int frameHeight)
    : m_texture(texture), m_frameCount(frameCount > 0 ? frameCount : 1),
      m_startCol(startFrame), m_row(0), m_currentFrame(0),
      m_frameSpeed(frameSpeed), m_timer(0.0f),
      m_frameWidth(frameWidth), m_frameHeight(frameHeight), m_loop(true) {}

Animation::Animation(Texture2D* texture, int frameCount, float frameSpeed, int startCol, int row, int frameWidth, int frameHeight, bool loop)
    : m_texture(texture), m_frameCount(frameCount > 0 ? frameCount : 1),
      m_startCol(startCol), m_row(row), m_currentFrame(0),
      m_frameSpeed(frameSpeed), m_timer(0.0f),
      m_frameWidth(frameWidth), m_frameHeight(frameHeight), m_loop(loop) {}

void Animation::Update(float dt) {
    if (m_frameCount <= 1 || m_frameSpeed <= 0.0f) return;

    m_timer += dt;
    if (m_timer >= (1.0f / m_frameSpeed)) {
        m_timer = 0.0f;
        if (m_loop) {
            m_currentFrame = (m_currentFrame + 1) % m_frameCount;
        } else {
            m_currentFrame = std::min(m_currentFrame + 1, m_frameCount - 1);
        }
    }
}

void Animation::Draw(Vector2 position, Vector2 size, bool flipX, Color tint) const {
    if (!m_texture || m_texture->id == 0) {
        // Fallback drawing if texture is absent
        DrawRectangleV(position, size, tint);
        return;
    }

    int colIndex = m_startCol + m_currentFrame;
    Rectangle sourceRect = {
        static_cast<float>(colIndex * m_frameWidth),
        static_cast<float>(m_row * m_frameHeight),
        static_cast<float>(flipX ? -m_frameWidth : m_frameWidth),
        static_cast<float>(m_frameHeight)
    };

    Rectangle destRect = {
        position.x,
        position.y,
        size.x,
        size.y
    };

    Vector2 origin = { 0.0f, 0.0f };
    DrawTexturePro(*m_texture, sourceRect, destRect, origin, 0.0f, tint);
}

void Animation::Reset() {
    m_currentFrame = 0;
    m_timer = 0.0f;
}

void Animation::SetCurrentFrame(int frame) {
    if (m_frameCount > 0) {
        m_currentFrame = std::clamp(frame, 0, m_frameCount - 1);
    }
}
