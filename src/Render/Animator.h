#pragma once
//#include <iostream>
//#include "Core/Utils.h"
//#include "Engine/Sprite.h"
//
//class Animator {
//public:
//    Animator(
//        Sprite& sprite,
//		XMFLOAT3 pos,
//        Texture& texture,
//        XMFLOAT4 rect,
//        int numFrames,
//        float frameTime,
//        float scale,
//        int sizeBetweenFrame = 0,
//		XMFLOAT2 dir = XMFLOAT2(0, 0),
//        int rotation = 0
//    );
//
//    void update(float deltaTime);
//    int getCurrentFrame() const;
//    int getNumFrames() const;
//
//    bool isFinished() const;
//
//private:
//    Sprite& m_sprite;
//	XMFLOAT3 m_position;
//	XMFLOAT2 m_direction;
//    Texture& m_texture;
//    XMFLOAT4& m_frameRect;
//    int m_numFrames;
//    float m_frameTime;
//    float m_timeSinceLastFrame;
//    int m_currentFrame;
//    float m_scale;
//    int m_sizeBetweenFrame;
//    int m_spriteRotation;
//    bool m_isFinished;
//};
