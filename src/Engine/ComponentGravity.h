#pragma once
#include "Component.h"

class ComponentGravity : public Component
{
	float m_gravity = 9.81f;
	float m_mass = 0.5f;
	float m_terminalVelocity = 50.f;
public:
	ComponentGravity(Entity* e);
	~ComponentGravity() override;
	void Init() override;
	void Update(float dt) override;

	void SetGravity(float gravity) { m_gravity = gravity; }
	float GetGravity() const { return m_gravity; }

	void SetMass(float mass);
	float GetMass() const;

	void SetTerminalVelocity(float terminalVelocity);
	float GetTerminalVelocity() const;

	std::unique_ptr<Component> Clone() const override;

	void Serialize(nlohmann::json& j) const override;
	void Deserialize(const nlohmann::json& j) override;

	ComponentType* GetTag() override;
};

