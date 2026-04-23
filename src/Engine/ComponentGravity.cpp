#include "pch.h"
#include "ComponentGravity.h"

ComponentGravity::ComponentGravity(Entity* e) : Component(e)
{
	m_pEntity = e;
}

ComponentGravity::~ComponentGravity()
{

}

void ComponentGravity::Init()
{
}

void ComponentGravity::Update(float dt)
{
	if (m_mass * m_gravity >= m_terminalVelocity)
		m_mass = m_terminalVelocity / m_gravity;

	m_pEntity->m_transform.Translate(XMFLOAT3(0.f, -m_gravity * m_mass * dt, 0.f));
}

void ComponentGravity::SetMass(float mass)
{
	m_mass = mass;
}

float ComponentGravity::GetMass() const
{
	return m_mass;
}

void ComponentGravity::SetTerminalVelocity(float terminalVelocity)
{
	m_terminalVelocity = terminalVelocity;
}

float ComponentGravity::GetTerminalVelocity() const
{
	return m_terminalVelocity;
}

std::unique_ptr<Component> ComponentGravity::Clone() const
{
	return std::make_unique<ComponentGravity>(*this);
}

void ComponentGravity::Serialize(nlohmann::json& j) const
{
	j["type"] = "Gravity";
	j["gravity"] = m_gravity;
	j["mass"] = m_mass;
	j["terminalVelocity"] = m_terminalVelocity;
}

void ComponentGravity::Deserialize(const nlohmann::json& j)
{
	if (j.contains("gravity"))
		m_gravity = j["gravity"].get<float>();
	if (j.contains("mass"))
		m_mass = j["mass"].get<float>();
	if (j.contains("terminalVelocity"))
		m_terminalVelocity = j["terminalVelocity"].get<float>();
}

ComponentType* ComponentGravity::GetTag()
{
	m_type = ComponentType::Gravity;

	return &m_type;
}