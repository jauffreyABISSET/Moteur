#pragma once

#include "Transform.h"
#include "Scene.h"
#include "Component.h"
#include "Script.h"

class Component;
class Script;
class Prefab;

class Entity
{
public:
    Transform m_transform;

	Entity* m_parent = nullptr;
	std::vector<Entity*> m_children;

public:
    Entity(int id);
	Entity() = default;
    ~Entity();

    void InitComponents();
    void UpdateComponents(float dt);

    void InitScript();
    void UpdateScripts(float dt);
    void Destroy();

	int GetID() const;
    std::string GetName() const;
    void SetName(std::string name);

	void SetParent(Entity* parent);
	Entity* GetParent() const;
	bool HasParent() const;

	void AddChild(Entity* child);
	std::vector<Entity*> GetChildren() const;
    bool HasChild() const;

    bool HasScript() const;
	bool HasComponent() const;
    const std::vector<std::unique_ptr<Component>>& GetComponents() const;
    const std::vector<std::unique_ptr<Script>>& GetScripts() const;

    bool IsActive() const;
    void SetActive(bool active);

    bool IsDestroyed() const;
    void SetDestroyed(bool destroyed);

    const Prefab& GetRegisteredPrefabs(std::string prefabName) const;

    void Serialize(nlohmann::json& j) const;

    void AddComponent(std::unique_ptr<Component> comp);
	void AddScript(std::unique_ptr<Script> script);

    Component* AddComponentByName(const std::string& typeName);
	void AddScriptByName(const std::string& typeName);

    bool HasTexture() const;
	const std::string GetTextureName() const;

    template <typename COMPONENT> bool HasComponent() const;
    template <typename COMPONENT> const COMPONENT* GetComponent() const;
    template <typename COMPONENT> COMPONENT* GetComponent();
    template <typename COMPONENT> COMPONENT* AddComponent();
    template <typename COMPONENT> void RemoveComponent();

	template <typename SCRIPT> bool HasScript() const;
	template <typename SCRIPT> const SCRIPT* GetScript() const;
	template <typename SCRIPT> SCRIPT* GetScript();
	template <typename SCRIPT> SCRIPT* AddScript();
	template <typename SCRIPT> void RemoveScript();

	std::unique_ptr<Entity> Clone() const;

private:
    bool m_created = false;
    bool m_destroyed = false;

    bool m_active = true;

    int m_id;
    std::string m_name;

    std::vector<std::unique_ptr<Component>> m_components{};
    std::vector<std::unique_ptr<Script>> m_scripts{};

    friend class Transform;
};

#include "Entity.inl"