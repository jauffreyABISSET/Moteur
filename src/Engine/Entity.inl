#include "Entity.h"

template<typename COMPONENT>
inline bool Entity::HasComponent() const
{
    if (GetComponent<COMPONENT>())
        return true;

    return false;
}

template<typename COMPONENT>
inline COMPONENT* Entity::AddComponent()
{
    auto component = std::make_unique<COMPONENT>(this);
    COMPONENT* raw = component.get();
    m_components.push_back(std::move(component));

    if (raw) raw->Init();
    return raw;
}

template<typename COMPONENT>
inline const COMPONENT* Entity::GetComponent() const
{
    for (const auto& c : m_components) {
        if (auto casted = dynamic_cast<COMPONENT*>(c.get())) {
            return casted;
        }
    }
    return nullptr;
}

template<typename COMPONENT>
inline COMPONENT* Entity::GetComponent()
{
    for (const auto& c : m_components) {
        if (auto casted = dynamic_cast<COMPONENT*>(c.get())) {
            return casted;
        }
    }
    return nullptr;
}

template<typename COMPONENT>
inline void Entity::RemoveComponent()
{
    for (const auto& c : m_components) {
        if (dynamic_cast<COMPONENT*>(c.get())) {
            m_components.erase(std::remove(m_components.begin(), m_components.end(), c), m_components.end());
            return;
		}
    }
}

template<typename SCRIPT>
inline bool Entity::HasScript() const
{
    if (GetScript<SCRIPT>())
        return true;

    return false;
}

template<typename SCRIPT>
inline const SCRIPT* Entity::GetScript() const
{
    for (const auto& s : m_scripts) {
        if (auto casted = dynamic_cast<SCRIPT*>(s.get())) {
            return casted;
        }
    }
    return nullptr;
}

template<typename SCRIPT>
inline SCRIPT* Entity::GetScript()
{
    for (const auto& s : m_scripts) {
        if (auto casted = dynamic_cast<SCRIPT*>(s.get())) {
            return casted;
        }
    }
    return nullptr;
}

template <typename SCRIPT>
SCRIPT* Entity::AddScript()
{
    auto script = std::make_unique<SCRIPT>();
    SCRIPT* raw = script.get();

    if (raw) raw->SetEntity(this);

    m_scripts.push_back(std::move(script));

    if (raw) raw->Init();
    return raw;
}

template<typename SCRIPT>
inline void Entity::RemoveScript()
{
    for (const auto& s : m_scripts) {
        if (dynamic_cast<SCRIPT*>(s.get())) {
            m_scripts.erase(std::remove(m_scripts.begin(), m_scripts.end(), s), m_scripts.end());
            return;
        }
    }
}
