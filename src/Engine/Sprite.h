//#pragma once
//
//#include "Entity.h"
//#include "Transform.h"
////#include "Core/Utils.h"
//#include "Render/Texture.h"
//
//// A Sprite is an Entity with a texture and a position in the world.
//// It can be rendered on the screen using a SpriteRenderer component.
//class Sprite : public Entity
//{
//	Texture m_texture;
//	Transform m_transform;
//	XMFLOAT4 m_textureRect;
//
//public:
//	Sprite(int id, std::string path);
//	~Sprite() = default;
//
//	const Texture& GetTexture() const;
//	void SetTexture(const Texture& texture);
//	const Transform& GetTransform() const;
//	void SetTextureRect(XMFLOAT4 rect);
//	void SetScale(float scaleX, float scaleY);
//	void SetPosition(XMFLOAT3 position);
//	void SetDirection(XMFLOAT2 dir);
//	void SetRotation(int rot);
//	void Move(XMFLOAT2 delta);
//};