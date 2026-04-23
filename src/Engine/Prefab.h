#pragma once

class Entity;

class Prefab
{
public:
    static Prefab FromEntity(const Entity& e);

    void ApplyTo(Entity& target, bool asRoot = true, const XMFLOAT3* parentWorldPos = nullptr, const XMFLOAT4* parentWorldQuat = nullptr) const;

    const Transform& GetTransform() const;
    const std::string& GetName() const;
    const std::vector<Prefab>& GetChildren() const;

	void AddChild(Prefab child);
	void AddComponent(std::unique_ptr<Component> comp);
	void AddScript(std::unique_ptr<Script> script);

	Entity* GetParent() const;
	bool HasParent() const;
	void SetParent(Entity* parent);

	void Serialize(nlohmann::json& j) const;
	void Deserialize(const nlohmann::json& j);

	void ApplyComponents(Entity& target) const;
private:
    Transform m_transform;

    std::string m_name;
    Entity* m_parent = nullptr;
    std::vector<Prefab> m_children;

    std::vector<std::unique_ptr<Component>> m_components{};
    std::vector<std::unique_ptr<Script>>    m_scripts{};
};
