#pragma once

#include "GameTimer.h"

class Scene;

class SceneManager final
{
	std::vector<Scene*> m_scenes;
	std::vector<bool> m_sceneInitialized;
	std::vector<bool> m_sceneActive;

	int m_currentScene;
	int m_previousScene;

public:
	SceneManager();
	~SceneManager();

	virtual void Init();
	virtual void Update(const GameTimer& gt);
	virtual void Draw(const GameTimer& gt);

	void SwitchScene(int sceneId, bool init, bool stayActive);
	void SetSceneActive(int sceneId, bool active);

	void SetSceneActiveAll(bool active);
	void SetSceneActiveCurrent(bool active);

	Scene* GetCurrentScene() const;
	std::vector<Scene*> GetScenes() const;
	std::vector<Scene*> GetActiveScenes() const;

	template<typename T> void AddScene();
	template<typename T> void SwitchScene(bool init = true, bool stayActive = false);
	template<typename T> bool IsCurrentScene();
	template<typename T> T* GetScene();
	template<typename T> bool IsSceneActive();
	template<typename T> void SetSceneActive(bool active);
};

#include "SceneManager.inl"

