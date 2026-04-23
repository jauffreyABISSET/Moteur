#pragma once
#include "Component.h"
#include <utility>

class ComponentSphereCollider : public Component
{
	XMFLOAT3 m_lastPos{ 0.f, 0.f, 0.f };
	XMFLOAT3 m_center{ 0.f, 0.f, 0.f };
	XMFLOAT3 m_scale{ 1.f, 1.f, 1.f };
	XMFLOAT3 m_rotation{ 0.f, 0.f, 0.f };
	XMFLOAT3 m_radius{ 1.f, 1.f, 1.f };
	XMFLOAT3 m_offset{ 0.f, 0.f, 0.f };
	XMFLOAT3 m_distanceToCenter{ 0.f, 0.f, 0.f };

	float m_globalRadius{ 0.f };

public:
	ComponentSphereCollider(Entity* e);
	~ComponentSphereCollider() override;

	void Init() override;
	void Update(float dt) override;

	bool HasMoved() const;

	void SetOffset(const XMFLOAT3& offset = { 0.f,0.f,0.f });
	void SetRadius(const XMFLOAT3& radius = { 1.f,1.f,1.f });

	float GetGlobalBounce() const;
	XMFLOAT3 GetDistanceToCenter() const;
	XMFLOAT3 GetCenter() const;
	XMFLOAT3 GetRadius() const;

	std::pair<XMFLOAT3, float> ComputeWorldSphere() const;

	Entity* GetEntity() const { return m_pEntity; }

	std::unique_ptr<Component> Clone() const override;

	ComponentType* GetTag() override;

	void Serialize(nlohmann::json& j) const override;
	void Deserialize(const nlohmann::json& j) override;

	std::string GetName() const { return m_name; }
private:
	std::string m_name = "SphereCollider";
};

