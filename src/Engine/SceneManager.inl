#include "SceneManager.h" 
#include "Scene.h" 
#include <cassert>

template<typename T>
inline void SceneManager::AddScene()
{
	static_assert(std::is_base_of<Scene, T>::value, "T must be derived from Scene");
	Scene* newScene = new T();
	m_scenes.push_back(newScene);
	m_sceneInitialized.push_back(false);
	m_sceneActive.push_back(false);

	if (m_currentScene < 0)
		m_currentScene = 0;
}

template<typename T>
inline void SceneManager::SwitchScene(bool init, bool stayActive)
{
	static_assert(std::is_base_of<Scene, T>::value, "T must be derived from Scene");

	for (size_t i = 0; i < m_scenes.size(); ++i)
		if (dynamic_cast<T*>(m_scenes[i]) != nullptr) {
			SwitchScene(static_cast<int>(i), init, stayActive);
			return;
		}

	assert(false && "Scene of type T not found in SceneManager");
	return;
}

template<typename T>
inline bool SceneManager::IsCurrentScene()
{
	static_assert(std::is_base_of<Scene, T>::value, "T must be derived from Scene");

	if (m_currentScene < 0 || m_currentScene >= static_cast<int>(m_scenes.size()))
		return false;
	return dynamic_cast<T*>(m_scenes[m_currentScene]) != nullptr;
}

template<typename T>
inline T* SceneManager::GetScene()
{
	static_assert(std::is_base_of<Scene, T>::value, "T must be derived from Scene");

	for (size_t i = 0; i < m_scenes.size(); ++i)
		if (dynamic_cast<T*>(m_scenes[i]) != nullptr) {
			return dynamic_cast<T*>(m_scenes[i]);
		}

	assert(false && "Scene of type T not found in SceneManager");
	return nullptr;
}

template<typename T>
inline bool SceneManager::IsSceneActive()
{
	static_assert(std::is_base_of<Scene, T>::value, "T must be derived from Scene");

	for (size_t i = 0; i < m_scenes.size(); ++i)
		if (dynamic_cast<T*>(m_scenes[i]) != nullptr)
			return m_sceneActive[i];
		
	assert(false && "Scene of type T not found in SceneManager");

	return false;
}

template<typename T>
inline void SceneManager::SetSceneActive(bool active)
{
	static_assert(std::is_base_of<Scene, T>::value, "T must be derived from Scene");

	for (size_t i = 0; i < m_scenes.size(); ++i)
		if (dynamic_cast<T*>(m_scenes[i]) != nullptr) {
			m_sceneActive[i] = active;
			return;
		}

	assert(false && "Scene of type T not found in SceneManager");
}
