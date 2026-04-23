#include "pch.h"
#include "ComponentSphereCollider.h"

using namespace DirectX;

ComponentSphereCollider::ComponentSphereCollider(Entity* e) : Component(e)
{
}

ComponentSphereCollider::~ComponentSphereCollider()
{
}

void ComponentSphereCollider::Init()
{
    auto pr = ComputeWorldSphere();
    m_center = pr.first;
    m_globalRadius = pr.second;

    m_radius.x = m_pEntity->m_transform.GetScale().x * 0.5f;
    m_radius.y = m_pEntity->m_transform.GetScale().y * 0.5f;
    m_radius.z = m_pEntity->m_transform.GetScale().z * 0.5f;

    if (m_pEntity) {
        XMMATRIX world = m_pEntity->m_transform.GetWorldMatrix();

        XMFLOAT3 pos = {0.f, 0.f, 0.f};
        pos.x = world.r[3].m128_f32[0];
        pos.y = world.r[3].m128_f32[1];
        pos.z = world.r[3].m128_f32[2];
        m_lastPos = pos;
    }
}


bool ComponentSphereCollider::HasMoved() const
{
    if (!m_pEntity)
        return false;

    XMMATRIX world = m_pEntity->m_transform.GetWorldMatrix();
    XMFLOAT3 currentPos = { 0.f, 0.f, 0.f };
    currentPos.x = world.r[3].m128_f32[0];
    currentPos.y = world.r[3].m128_f32[1];
    currentPos.z = world.r[3].m128_f32[2];

    XMFLOAT3 currentRot = m_pEntity->m_transform.GetRotation();
    XMFLOAT3 currentScale = m_pEntity->m_transform.GetScale();

    return (currentPos.x != m_lastPos.x) || (currentPos.y != m_lastPos.y) || (currentPos.z != m_lastPos.z)
        || (currentRot.x != m_rotation.x) || (currentRot.y != m_rotation.y) || (currentRot.z != m_rotation.z)
        || (currentScale.x != m_scale.x) || (currentScale.y != m_scale.y) || (currentScale.z != m_scale.z);
}

void ComponentSphereCollider::Update(float dt)
{
    if (!HasMoved())
        return;

    auto pr = ComputeWorldSphere();
    m_center = pr.first;
    m_globalRadius = pr.second;

    m_radius.x = m_pEntity->m_transform.GetScale().x * 0.5f;
    m_radius.y = m_pEntity->m_transform.GetScale().y * 0.5f;
    m_radius.z = m_pEntity->m_transform.GetScale().z * 0.5f;

    if (m_pEntity) {
        XMMATRIX world = m_pEntity->m_transform.GetWorldMatrix();

        XMFLOAT3 pos = { 0.f, 0.f, 0.f };
        pos.x = world.r[3].m128_f32[0];
        pos.y = world.r[3].m128_f32[1];
        pos.z = world.r[3].m128_f32[2];
        m_lastPos = pos;
    }
}

std::pair<XMFLOAT3, float> ComponentSphereCollider::ComputeWorldSphere() const
{
    if (!m_pEntity)
        return { XMFLOAT3{0.f,0.f,0.f}, 0.f };

    // centre local + offset
    XMFLOAT3 localCenter = m_center;
    localCenter.x += m_offset.x;
    localCenter.y += m_offset.y;
    localCenter.z += m_offset.z;

    XMMATRIX world = m_pEntity->m_transform.GetWorldMatrix();

    // centre monde
    XMVECTOR vLocalCenter = XMLoadFloat3(&localCenter);
    XMVECTOR vWorldCenter = XMVector3TransformCoord(vLocalCenter, world);
    XMFLOAT3 worldCenter;
    XMStoreFloat3(&worldCenter, vWorldCenter);

    XMFLOAT3 trScale = m_pEntity->m_transform.GetScale();
    float maxScale = XMMax(trScale.x, XMMax(trScale.y, trScale.z));
    float worldRadius = m_radius.x * maxScale;

    if (worldRadius < 0.f) worldRadius = 0.f;

    return { worldCenter, worldRadius };
}

std::unique_ptr<Component> ComponentSphereCollider::Clone() const
{
	return std::make_unique<ComponentSphereCollider>(*this);
}


float ComponentSphereCollider::GetGlobalBounce() const
{
    return m_globalRadius;
}

XMFLOAT3 ComponentSphereCollider::GetDistanceToCenter() const
{
    return m_distanceToCenter;
}

XMFLOAT3 ComponentSphereCollider::GetCenter() const
{
    return m_center;
}

XMFLOAT3 ComponentSphereCollider::GetRadius() const
{
	return { m_radius.x, m_radius.y, m_radius.z };
}

void ComponentSphereCollider::SetOffset(const XMFLOAT3& offset)
{
    m_offset = offset;
}

void ComponentSphereCollider::SetRadius(const XMFLOAT3& radius)
{
    m_radius = radius;
}

ComponentType* ComponentSphereCollider::GetTag()
{
    m_type = ComponentType::SphereCollider; 
    
    return &m_type;
}

void ComponentSphereCollider::Serialize(nlohmann::json& j) const
{
    j["type"] = "SphereCollider";
    j["offset"] = { m_offset.x, m_offset.y, m_offset.z };
	j["radius"] = { m_radius.x, m_radius.y, m_radius.z };
}

void ComponentSphereCollider::Deserialize(const nlohmann::json& j)
{
    if (j.contains("offset") && j["offset"].is_array() && j["offset"].size() == 3) {
        m_offset.x = j["offset"][0].get<float>();
        m_offset.y = j["offset"][1].get<float>();
        m_offset.z = j["offset"][2].get<float>();
    }
    if (j.contains("radius") && j["radius"].is_array() && j["radius"].size() == 3) {
        m_radius.x = j["radius"][0].get<float>();
        m_radius.y = j["radius"][1].get<float>();
        m_radius.z = j["radius"][2].get<float>();
	}
}
