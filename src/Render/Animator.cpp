#include "pch.h"
//#include "Animator.h"
//
//Animator::Animator(Sprite& sprite,XMFLOAT3 pos, Texture& texture, XMFLOAT4 rect, int numFrames, float frameTime, float scale, int sizeBetweenFrame, XMFLOAT2 dir, int rot) :
//      m_sprite(sprite),
//      m_position(pos),
//      m_texture(texture),
//      m_frameRect(rect),
//      m_numFrames(numFrames),
//      m_frameTime(frameTime),
//      m_timeSinceLastFrame(0.0f),
//      m_currentFrame(0),
//      m_scale(scale),
//      m_sizeBetweenFrame(sizeBetweenFrame),
//      m_isFinished(false),
//	  m_direction(dir),
//	  m_spriteRotation(rot)
//{
//	m_sprite.SetTexture(texture);
//    m_sprite.SetTextureRect(m_frameRect);
//    m_sprite.SetScale(m_scale, m_scale);
//    m_sprite.SetPosition(m_position);
//    m_sprite.SetDirection(m_direction);
//    m_sprite.SetRotation(m_spriteRotation);
//}
//
//void Animator::update(float deltaTime) {
//    m_timeSinceLastFrame += deltaTime;
//
//    m_sprite.Move(m_direction);
//
//    if (m_timeSinceLastFrame >= m_frameTime) {
//        m_timeSinceLastFrame = 0.0f;
//
//        m_currentFrame++;
//
//        if (m_currentFrame >= m_numFrames) {
//            m_currentFrame = 0;
//            m_isFinished = true;
//        }
//
//        int x = m_frameRect.x + (m_currentFrame /*m_sprite.GetTransform().GetSize()*/) + m_sizeBetweenFrame;
//        int y = m_frameRect.y;
//
//		XMFLOAT4 newRect(x, y, x + (m_frameRect.z - m_frameRect.x), y + (m_frameRect.w - m_frameRect.y));
//        m_sprite.SetTextureRect(newRect);
//    }
//}
//
//int Animator::getCurrentFrame() const {
//    return m_currentFrame;
//}
//
//int Animator::getNumFrames() const {
//    return m_numFrames;
//}
//bool Animator::isFinished() const {
//    return m_isFinished;
//}
