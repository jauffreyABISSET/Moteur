#pragma once
#include "Component.h"

class ComponentRigidBody : public Component
{
private:
	float m_mass = 1.f;
	XMFLOAT3 m_velocity{ 0.f, 0.f, 0.f };
	bool m_isKinematic = false;

public:	
	ComponentRigidBody(Entity* e);
	~ComponentRigidBody() override;

	void Init() override;
	void Update(float dt) override;

	void SetMass(float mass) { m_mass = mass; }
	void SetVelocity(const XMFLOAT3& velocity) { m_velocity = velocity; }
	void AddForce(const XMFLOAT3& force);

	void SetKinematic(bool isKinematic) { m_isKinematic = isKinematic; }
	bool IsKinematic() const { return m_isKinematic; }
	
	std::unique_ptr<Component> Clone() const override;

	ComponentType* GetTag() override;
	void Serialize(nlohmann::json& j) const override;
	void Deserialize(const nlohmann::json& j) override;

	std::string GetName() const { return m_name; }
private:
	std::string m_name = "RigidBody";
};

