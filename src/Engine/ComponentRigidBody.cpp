#include "pch.h"
#include "ComponentRigidBody.h"

ComponentRigidBody::ComponentRigidBody(Entity* e) : Component(e)
{
	m_pEntity = e;
}

ComponentRigidBody::~ComponentRigidBody()
{

}

void ComponentRigidBody::Init()
{

}

void ComponentRigidBody::Update(float dt)
{

}

void ComponentRigidBody::AddForce(const XMFLOAT3& force)
{
	if (m_isKinematic)
		return;
	m_velocity.x += force.x / m_mass;
	m_velocity.y += force.y / m_mass;
	m_velocity.z += force.z / m_mass;
}

std::unique_ptr<Component> ComponentRigidBody::Clone() const
{
	return std::make_unique<ComponentRigidBody>(*this);
}

ComponentType* ComponentRigidBody::GetTag()
{
	m_type = ComponentType::RigidBody; 
	return &m_type;
}

void ComponentRigidBody::Serialize(nlohmann::json& j) const
{
	j["type"] = "RigidBody";
	j["mass"] = m_mass;
	j["velocity"] = { m_velocity.x, m_velocity.y, m_velocity.z };
	j["isKinematic"] = m_isKinematic;
}

void ComponentRigidBody::Deserialize(const nlohmann::json& j)
{
	if (j.contains("mass"))
		m_mass = j["mass"].get<float>();

	if (j.contains("velocity") && j["velocity"].is_array() && j["velocity"].size() == 3) {
		m_velocity.x = j["velocity"][0].get<float>();
		m_velocity.y = j["velocity"][1].get<float>();
		m_velocity.z = j["velocity"][2].get<float>();
	}

	if (j.contains("isKinematic"))
		m_isKinematic = j["isKinematic"].get<bool>();
}
