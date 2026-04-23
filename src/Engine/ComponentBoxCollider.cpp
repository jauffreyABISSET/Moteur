#include "pch.h"
#include "ComponentBoxCollider.h"

using namespace DirectX;

ComponentBoxCollider::ComponentBoxCollider(Entity* e)
    : Component(e)
{

}

ComponentBoxCollider::~ComponentBoxCollider()
{

}

void ComponentBoxCollider::Init()
{
    m_globalBounce = ComputeWorldAABB();

    if (m_pEntity) {
        XMMATRIX world = m_pEntity->m_transform.GetWorldMatrix();
        XMMATRIX rotationMatrix = m_pEntity->m_transform.GetMatrixRotation();

        XMFLOAT3 pos;
        pos.x = world.r[3].m128_f32[0];
        pos.y = world.r[3].m128_f32[1];
        pos.z = world.r[3].m128_f32[2];
        m_lastPos = pos;

        m_scale = m_pEntity->m_transform.GetScale();
		m_offset = m_pEntity->m_transform.GetOffset();
    }
}

bool ComponentBoxCollider::HasMoved() const
{
    if (!m_pEntity)
        return false;

    XMMATRIX world = m_pEntity->m_transform.GetWorldMatrix();
    XMFLOAT3 currentPos;
    currentPos.x = world.r[3].m128_f32[0];
    currentPos.y = world.r[3].m128_f32[1];
    currentPos.z = world.r[3].m128_f32[2];

    XMFLOAT3 currentRot = m_pEntity->m_transform.GetRotation();
    XMFLOAT3 currentScale = m_pEntity->m_transform.GetScale();

    return (currentPos.x != m_lastPos.x) || (currentPos.y != m_lastPos.y) || (currentPos.z != m_lastPos.z)
        || (currentRot.x != m_rotation.x) || (currentRot.y != m_rotation.y) || (currentRot.z != m_rotation.z)
        || (currentScale.x != m_scale.x) || (currentScale.y != m_scale.y) || (currentScale.z != m_scale.z);
}

void ComponentBoxCollider::Update(float dt)
{
    if (!HasMoved())
        return;

    m_scale = m_pEntity->m_transform.GetScale();
    m_rotation = m_pEntity->m_transform.GetRotation();
    m_globalBounce = ComputeWorldAABB();

    XMMATRIX world = m_pEntity->m_transform.GetWorldMatrix();
    XMFLOAT3 worldPos;
    worldPos.x = world.r[3].m128_f32[0];
    worldPos.y = world.r[3].m128_f32[1];
    worldPos.z = world.r[3].m128_f32[2];

    // center du AABB
    XMFLOAT3 min = m_globalBounce.first;
    XMFLOAT3 max = m_globalBounce.second;
    XMFLOAT3 center{ (min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f, (min.z + max.z) * 0.5f };

    m_distanceToCenter = XMFLOAT3{ center.x - worldPos.x, center.y - worldPos.y, center.z - worldPos.z };
    m_lastPos = worldPos;

    m_rotation = m_pEntity->m_transform.GetRotation();
    m_scale = m_pEntity->m_transform.GetScale();

}

void ComponentBoxCollider::SetSize(const XMFLOAT3& size)
{
    m_size = size;
}

XMFLOAT3 ComponentBoxCollider::GetSize() const
{
    return m_size;
}

void ComponentBoxCollider::SetRotation(const XMFLOAT3& rotation)
{
    m_rotation = rotation;
}

void ComponentBoxCollider::SetOffset(const XMFLOAT3& offset)
{
    m_offset = offset;
}

XMFLOAT3 ComponentBoxCollider::GetOffset() const
{
    return m_offset;
}

std::pair<XMFLOAT3, XMFLOAT3> ComponentBoxCollider::GetGlobalBounce() const
{
    return m_globalBounce;
}

XMFLOAT3 ComponentBoxCollider::GetDistanceToCenter() const
{
    return m_distanceToCenter;
}

std::pair<XMFLOAT3, XMFLOAT3> ComponentBoxCollider::ComputeWorldAABB() const
{
    if (!m_pEntity)
        return { XMFLOAT3{0,0,0}, XMFLOAT3{0,0,0} };

    // local center + offset
    XMFLOAT3 localCenter = m_center;
    localCenter.x += m_offset.x;
    localCenter.y += m_offset.y;
    localCenter.z += m_offset.z;

    XMFLOAT3 localHalf{
        (m_size.x) * m_scale.x,
        (m_size.y) * m_scale.y,
        (m_size.z) * m_scale.z
    };

    // matrices
    XMMATRIX world = m_pEntity->m_transform.GetWorldMatrix();
    XMMATRIX rotMat = m_pEntity->m_transform.GetMatrixRotation();

    // world center
    XMVECTOR vLocalCenter = XMLoadFloat3(&localCenter);
    XMVECTOR vWorldCenter = XMVector3TransformCoord(vLocalCenter, world);
    XMFLOAT3 worldCenter;
    XMStoreFloat3(&worldCenter, vWorldCenter);

    XMVECTOR hx = XMVectorSet(localHalf.x, 0.f, 0.f, 0.f);
    XMVECTOR hy = XMVectorSet(0.f, localHalf.y, 0.f, 0.f);
    XMVECTOR hz = XMVectorSet(0.f, 0.f, localHalf.z, 0.f);

    XMVECTOR wx = XMVector3TransformNormal(hx, rotMat);
    XMVECTOR wy = XMVector3TransformNormal(hy, rotMat);
    XMVECTOR wz = XMVector3TransformNormal(hz, rotMat);

    XMVECTOR verts[8];
    verts[0] = vWorldCenter + wx + wy + wz;
    verts[1] = vWorldCenter + wx + wy - wz;
    verts[2] = vWorldCenter + wx - wy + wz;
    verts[3] = vWorldCenter + wx - wy - wz;
    verts[4] = vWorldCenter - wx + wy + wz;
    verts[5] = vWorldCenter - wx + wy - wz;
    verts[6] = vWorldCenter - wx - wy + wz;
    verts[7] = vWorldCenter - wx - wy - wz;

    XMFLOAT3 v0; XMStoreFloat3(&v0, verts[0]);
    XMFLOAT3 min = v0;
    XMFLOAT3 max = v0;

    for (int i = 1; i < 8; ++i)
    {
        XMFLOAT3 vi; XMStoreFloat3(&vi, verts[i]);
        min.x = XMMin(min.x, vi.x);
        min.y = XMMin(min.y, vi.y);
        min.z = XMMin(min.z, vi.z);

        max.x = XMMax(max.x, vi.x);
        max.y = XMMax(max.y, vi.y);
        max.z = XMMax(max.z, vi.z);
    }

    return { min, max };
}

std::unique_ptr<Component> ComponentBoxCollider::Clone() const
{
    auto clone = std::make_unique<ComponentBoxCollider>(m_pEntity);

    return clone;
}

void ComponentBoxCollider::Serialize(nlohmann::json& j) const
{
    j["type"] = "BoxCollider";
    j["size"] = { m_size.x, m_size.y, m_size.z };
    j["offset"] = { m_offset.x, m_offset.y, m_offset.z };
    j["rotation"] = { m_rotation.x, m_rotation.y, m_rotation.z };
}

void ComponentBoxCollider::Deserialize(const nlohmann::json& j)
{
    if (j.contains("size") && j["size"].is_array() && j["size"].size() == 3) {
        m_size.x = j["size"][0].get<float>();
        m_size.y = j["size"][1].get<float>();
        m_size.z = j["size"][2].get<float>();
    }
    if (j.contains("offset") && j["offset"].is_array() && j["offset"].size() == 3) {
        m_offset.x = j["offset"][0].get<float>();
        m_offset.y = j["offset"][1].get<float>();
        m_offset.z = j["offset"][2].get<float>();
    }
    if (j.contains("rotation") && j["rotation"].is_array() && j["rotation"].size() == 3) {
        m_rotation.x = j["rotation"][0].get<float>();
        m_rotation.y = j["rotation"][1].get<float>();
        m_rotation.z = j["rotation"][2].get<float>();
	}
}

ComponentType* ComponentBoxCollider::GetTag()
{
    m_type = ComponentType::BoxCollider;
    return &m_type;
} 