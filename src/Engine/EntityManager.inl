#include "EntityManager.h"


// Get all entities that have a specific component type 
template<typename COMPONENT>
inline const std::vector<Entity*> EntityManager::GetEntitiesWithComponent() const
{
	std::vector<Entity*> result;

	for (const Entity* e : m_entities) {
		if (e->HasComponent<COMPONENT>()) {
			result.push_back(const_cast<Entity*>(e));
		}
	}
	return result;
}

