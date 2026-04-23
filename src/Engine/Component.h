#pragma once

#include <memory>
#include <../libs/J-SON/include/nlohmann/json.hpp>

class Entity;

enum class ComponentType
{
    MeshRenderer,
    Light,
    Physic,
    BoxCollider,
    SphereCollider,
    RigidBody,
    Gravity,
    Collider,
    Text,
    EnemyIA,

    NONE
};

class Component
{
protected:
    Entity* m_pEntity = nullptr;
	ComponentType m_type;

public:
    Component(Entity* e = nullptr);
    virtual ~Component();

    virtual void Init() = 0;
    virtual void Update(float dt) = 0;
    virtual std::unique_ptr<Component> Clone() const = 0;
	virtual ComponentType* GetTag() = 0;
    virtual void Serialize(nlohmann::json& j) const = 0;
    virtual void Deserialize(const nlohmann::json& j) = 0;

    void SetEntity(Entity* e);

    Entity* GetOwner();
};