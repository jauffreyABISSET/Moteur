#include "pch.h"
#include "Component.h"

Component::Component(Entity* e)
{
    m_type = ComponentType::NONE;
    SetEntity(e);
}

Component::~Component()
{
}

void Component::SetEntity(Entity* e)
{
    m_pEntity = e;
}

Entity* Component::GetOwner()
{
    return m_pEntity;
}