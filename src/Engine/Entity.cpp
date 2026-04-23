#include "pch.h"
#include "Entity.h"

Entity::Entity(int id)
{
	m_id = id;

	m_created = true;
}

Entity::~Entity()
{

}

void Entity::Destroy()
{
	m_destroyed = true;
}

void Entity::AddComponent(std::unique_ptr<Component> comp)
{
	if (!comp)
		return;

	comp->SetEntity(this);

	Component* raw = comp.get();
	m_components.push_back(std::move(comp));

	if (raw)
		raw->Init();
}


void Entity::AddScript(std::unique_ptr<Script> script)
{
	if (!script)
		return;

	script->SetEntity(this);

	Script* raw = script.get();
	m_scripts.push_back(std::move(script));

	if (raw) {
		raw->Init();
	}
}

Component* Entity::AddComponentByName(const std::string& typeName)
{
	Component* comp = nullptr;
	if (typeName == "MeshRenderer") {
		comp = new ComponentMeshRenderer(this);
	}

	if (typeName == "SphereCollider") {
		comp = new ComponentSphereCollider(this);
	}

	if (typeName == "RigidBody") {
		comp = new ComponentRigidBody(this);
	}

	if (typeName == "Gravity") {
		comp = new ComponentGravity(this);
	}

	if (typeName == "BoxCollider") {
		comp = new ComponentBoxCollider(this);
	}

	if (typeName == "Light") {
		comp = new ComponentLight(this);
	}

	if (typeName == "EnemyIA") {
		comp = new ComponentLight(this);
	}

	if (comp == nullptr) {
		std::cerr << "Component type '" << typeName << "' not recognized." << std::endl;
		return nullptr;
	}

	if (comp) {
		m_components.emplace_back(comp);
	}
	return comp;
}

void Entity::AddScriptByName(const std::string& typeName)
{
	Script* script = nullptr;
	script->GetScripts().find(typeName) != script->GetScripts().end() ? script->GetScripts()[typeName] : nullptr;

	if (script == nullptr) {
		std::cerr << "Script type '" << typeName << "' not recognized." << std::endl;
		return;
	}

	if (script) {
		m_scripts.push_back(std::move(std::unique_ptr<Script>(script)));
		script->SetEntity(this);
		script->Init();
	}
}

bool Entity::HasTexture() const
{
	if (this->HasComponent<ComponentMeshRenderer>()) {
		return this->GetComponent<ComponentMeshRenderer>()->GetTexture() != nullptr;
	}
	return false;
}

const std::string Entity::GetTextureName() const
{
	if (HasTexture())
		return this->GetComponent<ComponentMeshRenderer>()->GetTexture()->GetName();
}

void Entity::InitComponents()
{
	for (auto& c : m_components)
		if (c) c->Init();
}

void Entity::UpdateComponents(float dt)
{
	for (auto& c : m_components)
		if (c) c->Update(dt);
}

void Entity::InitScript()
{
	for (auto& c : m_scripts)
		if (c) c->Init();
}

void Entity::UpdateScripts(float dt)
{
	for (auto& c : m_scripts)
		if (c) c->Update(dt);
}

void Entity::SetActive(bool active)
{
	m_active = active;
}

bool Entity::IsActive() const
{
	return m_active;
}

void Entity::SetDestroyed(bool destroyed)
{
	m_destroyed = destroyed;
}

bool Entity::IsDestroyed() const
{
	return m_destroyed;
}

void Entity::SetName(std::string name)
{
	m_name = name;
}

std::string Entity::GetName() const
{
	return m_name;
}

int Entity::GetID() const
{
	return m_id;
}

const std::vector<std::unique_ptr<Component>>& Entity::GetComponents() const
{
	return m_components;
}

bool Entity::HasScript() const
{
	return !m_scripts.empty();
}

bool Entity::HasComponent() const
{
	return !m_components.empty();
}

const std::vector<std::unique_ptr<Script>>& Entity::GetScripts() const
{
	return m_scripts;
}

const Prefab& Entity::GetRegisteredPrefabs(std::string prefabName) const
{
	EntityManager& em = GameManager::GetInstance()->GetEntityManager();
	const auto& prefabs = em.GetRegisteredPrefabs();
	auto it = prefabs.find(prefabName);

	if (it != prefabs.end() && !it->second.empty()) {
		return it->second.front();
	}

	return it->second.front();
}

void Entity::Serialize(nlohmann::json& j) const
{
	j["name"] = m_name;
	j["active"] = m_active;

	j["position"] = {
		m_transform.GetWorldPosition().x,
		m_transform.GetWorldPosition().y,
		m_transform.GetWorldPosition().z
	};

	XMFLOAT4 q = m_transform.GetLocalRotationQuaternion();
	j["rotation"] = { q.x, q.y, q.z, q.w };

	j["scale"] = {
		m_transform.GetScale().x,
		m_transform.GetScale().y,
		m_transform.GetScale().z
	};

	j["components"] = nlohmann::json::array();
	j["scripts"] = nlohmann::json::array();

	for (const auto& c : m_components)
	{
		Json jc;
		jc["type"] = std::to_string(typeid(*c).hash_code());
		c->Serialize(jc);
		j["components"].push_back(jc);
	}

	for (const auto& s : m_scripts)
	{
		Json js;
		js["type"] = std::to_string(typeid(*s).hash_code());
		s->Serialize(js);
		j["scripts"].push_back(js);
	}

	j["children"] = nlohmann::json::array(); 
	for (const auto& child : m_children)
	{
		Json jc;
		child->Serialize(jc);
		j["children"].push_back(jc);
	}

	j["parent"] = m_parent ? m_parent->GetID() : -1;
}

void Entity::SetParent(Entity* parent)
{
	m_parent = parent;
	
}

Entity* Entity::GetParent() const
{
	return m_parent;
}

bool Entity::HasParent() const
{
	return m_parent != nullptr;
}

void Entity::AddChild(Entity* child)
{
	child->SetParent(this);
	m_children.push_back(child);
}

std::vector<Entity*> Entity::GetChildren() const
{
	return m_children;
}

bool Entity::HasChild() const
{
	return !m_children.empty();
}

std::unique_ptr<Entity> Entity::Clone() const
{
    auto clone = std::make_unique<Entity>();
    clone->m_transform = m_transform;

    clone->m_name = m_name;
    clone->m_active = m_active;

    for (const auto& comp : m_components)
    {
        if (comp)
            clone->AddComponent(comp->Clone());
    }

    for (const auto& script : m_scripts)
    {
        if (script && script->GetScripts().find(script->GetName()) != script->GetScripts().end()) {
			clone->AddScript(script->GetScripts()[script->GetName()]->Clone());
		}
    }

    return clone;
}