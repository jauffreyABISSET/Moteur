#include "pch.h"
#include "ComponentCollider.h"
#include "GameManager.h"
#include "SceneManager.h"

static Entity tempEntity;

ComponentCollider::ComponentCollider(Entity * e) : Component(e)
{
    m_pEntity = e;
}

ComponentCollider::~ComponentCollider()
{
}

void ComponentCollider::Init()
{
}

bool ComponentCollider::Intersects(const Entity* other) const
{
    if (m_pEntity->HasComponent<ComponentBoxCollider>() && other->HasComponent<ComponentBoxCollider>())
    {
        auto aa = m_pEntity->GetComponent<ComponentBoxCollider>()->ComputeWorldAABB();
        auto bb = other->GetComponent<ComponentBoxCollider>()->ComputeWorldAABB();

        bool overlapX = (aa.first.x <= bb.second.x) && (aa.second.x >= bb.first.x);
        bool overlapY = (aa.first.y <= bb.second.y) && (aa.second.y >= bb.first.y);
        bool overlapZ = (aa.first.z <= bb.second.z) && (aa.second.z >= bb.first.z);

        return overlapX && overlapY && overlapZ;
    }
    else if (m_pEntity->HasComponent<ComponentSphereCollider>() && other->HasComponent<ComponentSphereCollider>())
    {
        XMFLOAT3 center1 = m_pEntity->m_transform.GetWorldPosition();
        XMFLOAT3 center2 = other->m_transform.GetWorldPosition();

        float radius1 = m_pEntity->GetComponent<ComponentSphereCollider>()->GetRadius().x;
        float radius2 = other->GetComponent<ComponentSphereCollider>()->GetRadius().x;

        float dx = center1.x - center2.x;
        float dy = center1.y - center2.y;
        float dz = center1.z - center2.z;

        float dist2 = dx * dx + dy * dy + dz * dz;
        float rSum = radius1 + radius2;

        if (dist2 <= rSum * rSum)
            return true;

        return false;
    }
    else if ((m_pEntity->HasComponent<ComponentSphereCollider>() && other->HasComponent<ComponentBoxCollider>()) ||
             (m_pEntity->HasComponent<ComponentBoxCollider>() && other->HasComponent<ComponentSphereCollider>()))
    {
        const Entity* sphereEntity = m_pEntity;
        const Entity* boxEntity = other;

        if (m_pEntity->HasComponent<ComponentSphereCollider>()) {
            sphereEntity = m_pEntity;
            boxEntity = other;
        }
        else {
            sphereEntity = other;
            boxEntity = m_pEntity;
        }
        auto boxAABB = boxEntity->GetComponent<ComponentBoxCollider>()->ComputeWorldAABB();
        auto sphereCenter = sphereEntity->m_transform.GetWorldPosition();
        XMFLOAT3 sphereRadius = sphereEntity->GetComponent<ComponentSphereCollider>()->GetRadius();

        XMFLOAT3 closestPoint;
        closestPoint.x = XMMax(boxAABB.first.x, XMMin(sphereCenter.x, boxAABB.second.x));
        closestPoint.y = XMMax(boxAABB.first.y, XMMin(sphereCenter.y, boxAABB.second.y));
        closestPoint.z = XMMax(boxAABB.first.z, XMMin(sphereCenter.z, boxAABB.second.z));

        float dx = closestPoint.x - sphereCenter.x;
        float dy = closestPoint.y - sphereCenter.y;
        float dz = closestPoint.z - sphereCenter.z;
        float dist2 = dx * dx + dy * dy + dz * dz;
        float r2 = sphereRadius.x * sphereRadius.x;

        if (dist2 <= r2)
            return true;

        return false;
    }
    return false;
}

std::unique_ptr<Component> ComponentCollider::Clone() const
{
    return std::make_unique<ComponentCollider>(*this);
}

void ComponentCollider::Serialize(nlohmann::json& j) const
{
    j["type"] = "Collider";
}

void ComponentCollider::Deserialize(const nlohmann::json& j)
{
    if (j.contains("type") && j["type"] == "Collider") {
		// No specific data to deserialize for the base Collider component, but this is where you would handle it if needed.
    }
}

ComponentType* ComponentCollider::GetTag()
{
    m_type = ComponentType::Collider; 
    
    return &m_type;
}

void ComponentCollider::Update(float dt)
{
    if (ignoreFirstFrame) {
        ignoreFirstFrame = false;
        return;
    }

    if (!m_pEntity) return;
    if (!m_pEntity->IsActive()) return;
    if (!m_pEntity->HasComponent<ComponentBoxCollider>() && !m_pEntity->HasComponent<ComponentSphereCollider>()) return;

    const auto& activeScenes = GameManager::GetInstance()->GetSceneManager().GetActiveScenes();

    for (Scene* scene : activeScenes)
    {
        if (!scene) continue;
        for (Entity* e : scene->GetEntitiesOfThisScene())
        {
            if (e == m_pEntity) continue;
            if (!e->IsActive()) continue;

            if (m_pEntity->HasComponent<ComponentRigidBody>())
                if (m_pEntity->GetComponent<ComponentRigidBody>()->IsKinematic() == true)
                    continue;

            if (!m_pEntity->HasComponent<ComponentBoxCollider>() && !m_pEntity->HasComponent<ComponentSphereCollider>()) continue;

            if (Intersects(e))
            {
                OnCollision();
            }
        }
    }
}

void ComponentCollider::OnCollision()
{
    m_pEntity->m_transform.SetPositionNoRecord(m_pEntity->m_transform.GetLastWorldPosition());

    if (m_pEntity->HasComponent<ComponentRigidBody>())
    {
        m_pEntity->GetComponent<ComponentRigidBody>()->SetVelocity({ 0.0f, 0.0f, 0.0f });
    }
}
