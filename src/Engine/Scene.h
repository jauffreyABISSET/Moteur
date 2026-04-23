#pragma once

#include "GameTimer.h"


class Entity;
class Prefab;

class Scene {
public:
    Scene() = default;
    virtual ~Scene() = default;
    virtual void Init() = 0;
    virtual void Update(const GameTimer& gt) = 0;
    virtual void Draw(const GameTimer& gt) = 0;

    void SetStateEntities(bool active);

    Entity* CreateEntity();
    void DestroyEntity(Entity* e);

    Entity* CreatePrefab(const std::string& prefabName);

    std::vector<Entity*> CreatePrefabs(const std::string& prefabName, XMFLOAT3 pos = { 0,0,0 }, XMFLOAT4 quat = { 0,0,0,1 }, bool returnAll = true);
    Entity* CreatePrefab(Prefab& prefab);

    std::vector<Entity*> CreatePrefabGroup(const std::string& groupName);

    std::vector<Entity*> GetEntitiesOfThisScene();
    std::unordered_map<std::string, const Prefab*> GetPrefabsOfThisScene() const;

    void RegisterPrefabGroup(const std::string& name, const std::vector<const Prefab*>& prefabs);

    const Prefab* GetPrefab(const std::string& name) const;
    Prefab* GetPrefab(const std::string& name);

	bool prefabExist(const std::string& name) const;
private :
    void RegisterEntity(Entity* e);
    void UnregisterEntity(Entity* e);

    void RegisterPrefab(const std::string& name, const Prefab* prefab);

protected:
    std::vector<Entity*> m_entities;
    std::unordered_map<std::string, const Prefab*> m_prefabs;
    std::unordered_map<std::string, std::vector<const Prefab*>> m_prefabGroups;
};
