#include "pch.h"
#include "RailEditors.h"

void RailEditorScript::Init()
{
    m_name = "RailEditorScript";
    if (m_pEntity)
    {
        m_initialPosition = m_pEntity->m_transform.GetLocalPosition();
        m_initialRotation = m_pEntity->m_transform.GetLocalRotationQuaternion();
    }
}

void RailEditorScript::Update(float dt)
{
    if (!m_pEntity) return;

    XMFLOAT3 rotationDelta{ 0.f, 0.f, 0.f };

    if (GetKey(Keyboard::LEFT))  rotationDelta.y -= m_rotationSpeed * dt;
    if (GetKey(Keyboard::RIGHT)) rotationDelta.y += m_rotationSpeed * dt;
    if (GetKey(Keyboard::UP))    rotationDelta.x -= m_pitchSpeed * dt;
    if (GetKey(Keyboard::DOWN))  rotationDelta.x += m_pitchSpeed * dt;

    rotationDelta.x = XMConvertToRadians(rotationDelta.x);
    rotationDelta.y = XMConvertToRadians(rotationDelta.y);
    rotationDelta.z = XMConvertToRadians(rotationDelta.z);

    m_pEntity->m_transform.Rotate(rotationDelta);

    if (GetKeyDown(Keyboard::R))
    {
        m_pEntity->m_transform.SetLastLocalPosition(m_initialPosition);
        m_pEntity->m_transform.SetLocalRotation(m_initialRotation);
        std::cout << "Rail reset to initial transform.\n";
    }
}

std::unique_ptr<Script> RailEditorScript::Clone() const
{
    return std::make_unique<RailEditorScript>(*this);
}