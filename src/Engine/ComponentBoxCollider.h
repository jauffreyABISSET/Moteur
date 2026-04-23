#pragma once
#include <DirectXMath.h>
#include <utility>
#include "Component.h"

using namespace DirectX;

class ComponentBoxCollider : public Component
{
	XMFLOAT3 m_lastPos{ 0.f, 0.f, 0.f };
	XMFLOAT3 m_center{ 0.f, 0.f, 0.f };
	XMFLOAT3 m_scale{ 1.f, 1.f, 1.f };
	XMFLOAT3 m_rotation{ 0.f, 0.f, 0.f };
	XMFLOAT3 m_size{ 1.f, 1.f, 1.f };
	XMFLOAT3 m_offset{ 0.f, 0.f, 0.f };
	XMFLOAT3 m_distanceToCenter{ 0.f, 0.f, 0.f };

	std::pair<XMFLOAT3, XMFLOAT3> m_globalBounce = {};
public:
	ComponentBoxCollider(Entity* e);
	~ComponentBoxCollider() override;

	void Init() override;
	bool HasMoved() const;
	void Update(float dt) override;

	void SetRotation(const XMFLOAT3& rotation = { 0.f,0.f,0.f });

	void SetSize(const XMFLOAT3& size = { 1.f,1.f,1.f });
	XMFLOAT3 GetSize() const;

	void SetOffset(const XMFLOAT3& offset = { 0.f,0.f,0.f });
	XMFLOAT3 GetOffset() const;

	std::pair<XMFLOAT3, XMFLOAT3> GetGlobalBounce() const;
	XMFLOAT3 GetDistanceToCenter() const;

	std::pair<XMFLOAT3, XMFLOAT3> ComputeWorldAABB() const;

	Entity* GetEntity() const { return m_pEntity; }

	std::unique_ptr<Component> Clone() const override;

	virtual void Serialize(nlohmann::json& j) const;
	void Deserialize(const nlohmann::json& j) override;

	ComponentType* GetTag() override;
};