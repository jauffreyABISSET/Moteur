#pragma once

class Entity;
class Prefab;

class EntityManager final
{
	std::vector<Entity*> m_entities;
	std::unordered_map<std::string, std::vector<Prefab>> m_prefabs;

	int m_nbId;

public:
	EntityManager();
	~EntityManager();

	Entity* CreateEntity();
	void DestroyEntity(Entity* e);

	const std::vector<Entity*>& GetEntities() const;
	const std::unordered_map<std::string, std::vector<Prefab>>& GetPrefabs() const;
	const std::string GetEntityNameByID(int id) const;
	const Entity* GetEntityByName(const std::string& name) const;
	const Entity* GetEntityByID(int id) const;

	template <typename COMPONENT>
	const std::vector<Entity*>GetEntitiesWithComponent() const;

	Entity* CreatePrefab(const Prefab& prefab);
	Entity* CreatePrefab(const std::string& prefabName);

	void RegisterPrefab(const std::string& name, Prefab prefab);
	void RegisterPrefab(const std::string& name, std::vector<Prefab> prefabs);

	void UnregisterPrefab(const std::string& name);
	const std::unordered_map<std::string, std::vector<Prefab>>& GetRegisteredPrefabs() const;
};

