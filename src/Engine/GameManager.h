#pragma once
#include <windows.h>
#include "EntityManager.h"

#include "SceneManager.h"
#include "AppManager.h"
#include "StateMachine.h"
#include "Render/LightManager.h"

class Window;
class RenderSystem;
class EntityManager;
class SceneManager;
class AppManager;
class LightManager;
class StateMachine;

class GameManager final
{
    static GameManager* s_pInstance;

    Window* m_pWindow = nullptr;
    RenderSystem* m_pRenderSystem = nullptr;

    EntityManager* m_pEntityManager = nullptr;
    SceneManager* m_pSceneManager = nullptr;
    AppManager* m_pAppManager = nullptr;
	LightManager* m_pLightManager = nullptr;

    bool m_isRunning = false;

	GameTimer m_deltaTime;

    std::unordered_map<Entity*, std::unique_ptr<StateMachine>> m_stateMachineList;

private:
    GameManager();
    ~GameManager();
public:
    static GameManager* GetInstance();

    void Init(HINSTANCE hinstance);
    void Run();
    void Destroy();

    StateMachine* CreateStateMachine(Entity* p_owner);

    Window& GetWindow();
	RenderSystem& GetRenderSystem();
    AppManager& GetAppManager();
    EntityManager& GetEntityManager();
    SceneManager& GetSceneManager();
	LightManager& GetLightManager();
    GameTimer& GetDeltaTime();
};