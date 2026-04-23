#include "pch.h"
#include "Scene.h"
#include "Engine/GameManager.h"
#include "Engine/EntityManager.h"

void Scene::SetStateEntities(bool active)
{
    for (Entity* e : m_entities)
        if (e) e->SetActive(active);
}

void Scene::RegisterEntity(Entity* e)
{
    auto it = std::find(m_entities.begin(), m_entities.end(), e);

    if (it == m_entities.end()) 
        m_entities.push_back(e);
}

void Scene::RegisterPrefab(const std::string& name, const Prefab* prefab)
{
    m_prefabs.insert_or_assign(name, prefab);
}

void Scene::RegisterPrefabGroup(const std::string& name, const std::vector<const Prefab*>& prefabs)
{
    m_prefabGroups.insert_or_assign(name, prefabs);
}

void Scene::UnregisterEntity(Entity* e)
{
    if (!e) return;
    auto it = std::find(m_entities.begin(), m_entities.end(), e);

    if (it != m_entities.end()) 
        m_entities.erase(it);
}

Entity* Scene::CreateEntity()
{
    Entity* e = GameManager::GetInstance()->GetEntityManager().CreateEntity();
    RegisterEntity(e);
    return e;
}

void Scene::DestroyEntity(Entity* e)
{
    if (!e) return;
    UnregisterEntity(e);
    GameManager::GetInstance()->GetEntityManager().DestroyEntity(e);
}

Entity* Scene::CreatePrefab(const std::string& prefabName)
{
    auto created = CreatePrefabs(prefabName);
    return created.empty() ? nullptr : created.front();
}

std::vector<Entity*> Scene::CreatePrefabs(const std::string& prefabName, XMFLOAT3 pos, XMFLOAT4 quat, bool returnAll)
{
    std::vector<Entity*> createdEntities;
    EntityManager& em = GameManager::GetInstance()->GetEntityManager();

    const auto& registeredMap = em.GetRegisteredPrefabs();
    auto it = registeredMap.find(prefabName);
    if (it == registeredMap.end() || it->second.empty())
        return createdEntities;

    if (it->second.size() == 1) {

        const Prefab& p = it->second.front();

        if (!p.GetChildren().empty())
        {
            // Parent or single Prefab
            Entity* parent = em.CreatePrefab(p);
            if (!parent) return createdEntities;

            parent->m_transform.SetPosition(pos);
            parent->m_transform.SetLocalRotation(quat);
            p.ApplyComponents(*parent);
            parent->InitComponents();
            parent->SetName(prefabName);
            parent->SetActive(true);
            RegisterEntity(parent);

            createdEntities.push_back(parent);

            std::vector<Entity*> existingChildren = parent->GetChildren();
            if (!existingChildren.empty()) {
                for (Entity* childEntity : existingChildren) {
                    if (!childEntity) continue;
                    childEntity->m_transform.SetPosition(pos);
                    childEntity->m_transform.AddLocalRotation(quat);
                    childEntity->SetName(prefabName);
                    p.ApplyComponents(*childEntity);
					childEntity->InitComponents();
                    childEntity->SetActive(true);
                    RegisterEntity(childEntity);
                    createdEntities.push_back(childEntity);
                }
            }
            else {
                for (const Prefab& child : p.GetChildren()) {
                    Entity* e = em.CreatePrefab(child);
                    if (e) {
                        // appliquer composants racine du child prefab ici aussi
                        child.ApplyComponents(*e);
                        e->InitComponents();
                        e->m_transform.SetPosition(pos);
                        e->m_transform.AddLocalRotation(quat);
                        e->SetName(prefabName);
                        e->SetParent(parent);
                        parent->AddChild(e);
                        e->SetActive(true);
                        RegisterEntity(e);
                        createdEntities.push_back(e);
                    }
                }
            }

            return createdEntities;
        }
        else {
            Entity* e = em.CreatePrefab(p);
            if (e) {
                // applique composants racine ici
                p.ApplyComponents(*e);
                e->InitComponents();
                e->m_transform.SetPosition(pos);
                e->m_transform.SetLocalRotation(quat);
                e->SetName(prefabName);
                e->SetActive(true);
                RegisterEntity(e);
                createdEntities.push_back(e);
            }
            return createdEntities;
        }
    }

    for (const Prefab& p : it->second) {
        Entity* e = em.CreatePrefab(p);
        if (e) {
            p.ApplyComponents(*e);
            e->m_transform.SetPosition(pos);
            e->m_transform.SetLocalRotation(quat);
            e->SetName(prefabName);
            e->SetActive(true);
            RegisterEntity(e);
            createdEntities.push_back(e);
        }
    }

    std::vector<const Prefab*> prefabPtrs;
    prefabPtrs.reserve(it->second.size());

    for (const Prefab& p : it->second) 
        prefabPtrs.push_back(&p);

    RegisterPrefabGroup(prefabName, prefabPtrs);

    return createdEntities;
}

std::vector<Entity*> Scene::CreatePrefabGroup(const std::string& groupName)
{
    std::vector<Entity*> createdEntities;

    auto it = m_prefabGroups.find(groupName);
    if (it == m_prefabGroups.end())
        return createdEntities;

    EntityManager& em = GameManager::GetInstance()->GetEntityManager();

    for (const Prefab* p : it->second)
    {
        if (!p) continue;
        Entity* e = em.CreatePrefab(*p);
        if (e)
        {
            RegisterEntity(e);
            createdEntities.push_back(e);
        }
    }

    return createdEntities;
}

Entity* Scene::CreatePrefab(Prefab& prefab)
{
    RegisterPrefab(prefab.GetName(), &prefab);

    EntityManager& em = GameManager::GetInstance()->GetEntityManager();
    Entity* created = em.CreatePrefab(prefab);

    RegisterEntity(created);

    return created;
}

std::vector<Entity*> Scene::GetEntitiesOfThisScene()
{
    return m_entities;
}

std::unordered_map<std::string, const Prefab*> Scene::GetPrefabsOfThisScene() const
{
    return m_prefabs;
}

const Prefab* Scene::GetPrefab(const std::string& name) const
{
    auto it = m_prefabs.find(name);
    if (it != m_prefabs.end()) 
        return it->second;

    auto itg = m_prefabGroups.find(name);
    if (itg != m_prefabGroups.end() && !itg->second.empty()) 
        return itg->second.front();

    return nullptr;
}

Prefab* Scene::GetPrefab(const std::string& name)
{
    const Prefab* p = static_cast<const Scene*>(this)->GetPrefab(name);
    return const_cast<Prefab*>(p);
}

bool Scene::prefabExist(const std::string& name) const
{
    for (const auto& kv : m_prefabs) {
        if (kv.first == name) {
            return true;
        }
	}
    return false;
}