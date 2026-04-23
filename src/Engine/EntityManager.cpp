#include "pch.h"
#include "EntityManager.h"
#include "Prefab.h"
#include "Entity.h"

EntityManager::EntityManager()
{
	m_nbId = 0;
}

EntityManager::~EntityManager()
{
	for (Entity* e : m_entities) {
		delete e;
	}
	m_entities.clear();
}

void EntityManager::DestroyEntity(Entity* e)
{
	if (!e) return;
	for (auto it = m_entities.begin(); it != m_entities.end(); ++it) {
		if (*it == e) {
			delete* it;
			m_entities.erase(it);
			return;
		}
	}
}

Entity* EntityManager::CreateEntity()
{
	Entity* e = new Entity(m_nbId);
	m_entities.push_back(e);
	++m_nbId;

	return e;
}

Entity* EntityManager::CreatePrefab(const Prefab& prefab)
{
	Entity* e = CreateEntity();
	prefab.ApplyTo(*e);
	return e;
}

Entity* EntityManager::CreatePrefab(const std::string& prefabName)
{
	auto it = m_prefabs.find(prefabName);
	if (it == m_prefabs.end() || it->second.empty())
		return nullptr;

	return CreatePrefab(it->second.front());
}

const std::unordered_map<std::string, std::vector<Prefab>>& EntityManager::GetPrefabs() const
{
	return m_prefabs;
}

const std::unordered_map<std::string, std::vector<Prefab>>& EntityManager::GetRegisteredPrefabs() const
{
	return m_prefabs;
}


void EntityManager::RegisterPrefab(const std::string& name, std::vector<Prefab> prefabs)
{
	m_prefabs.insert_or_assign(name, std::move(prefabs));
}


void EntityManager::RegisterPrefab(const std::string& name, Prefab prefab)
{
    auto& list = m_prefabs[name];
    list.push_back(std::move(prefab));
}

void EntityManager::UnregisterPrefab(const std::string& name)
{
	m_prefabs.erase(name);
}

const std::vector<Entity*>& EntityManager::GetEntities() const 
{
	return m_entities;
}

const std::string EntityManager::GetEntityNameByID(int id) const
{
	for (const Entity* e : m_entities) {
		if (e->GetID() == id) {
			return e->GetName();
		}
	}
	return "";
}

const Entity* EntityManager::GetEntityByName(const std::string& name) const
{
	for (const Entity* e : m_entities) {
		if (name == e->GetName()) {
			return e;
		}
	}
	return nullptr;
}

const Entity* EntityManager::GetEntityByID(int id) const
{
	for (const Entity* e : m_entities) {
		if (e->GetID() == id) {
			return e;
		}
	}
	return nullptr;
}
