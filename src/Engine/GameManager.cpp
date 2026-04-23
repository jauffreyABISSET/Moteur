#include "pch.h"
#include "GameManager.h"
#include "AppManager.h"
#include "SceneManager.h"
#include "EntityManager.h"

#include <thread>
#include <chrono>

GameManager* GameManager::s_pInstance = nullptr;

GameManager* GameManager::GetInstance()
{
    if (s_pInstance == nullptr) {
        s_pInstance = new GameManager();
    }
    return s_pInstance;
}

GameManager::GameManager()
    : m_pEntityManager(new EntityManager())
    , m_pSceneManager(new SceneManager())
    , m_pAppManager(nullptr)
    , m_pWindow(nullptr)
    , m_isRunning(true)
    , m_pRenderSystem(nullptr)
{
}

GameManager::~GameManager()
{
    if (this->m_pRenderSystem) {
        this->m_pRenderSystem = nullptr;
    }

	m_pAppManager->Destroy();
    m_pAppManager = nullptr;

    delete m_pEntityManager;
    m_pEntityManager = nullptr;

    delete m_pSceneManager;
    m_pSceneManager = nullptr;

    delete m_pWindow;
    m_pWindow = nullptr;

    m_isRunning = false;
}

void GameManager::Init(HINSTANCE hinstance)
{
    m_pWindow = new Window();
    m_pWindow->Init(hinstance, L"Game Engine", 1280, 720);

    m_pAppManager = AppManager::GetApp();
    m_pLightManager = LightManager::GetLM();

    if (m_pAppManager)
    {
        m_pAppManager->Initialize(m_pWindow);
        m_pAppManager->Init();
    }

    m_pRenderSystem = &RenderSystem::Get();


    if (m_pSceneManager)
    {
        m_pSceneManager->Init();
    }

    if (m_pLightManager)
    {
        m_pLightManager->CreateBuffer(m_pRenderSystem->GetDevice());
	}
}

GameTimer& GameManager::GetDeltaTime() {
	return m_deltaTime;
}

void GameManager::Run()
{
    if (s_pInstance == nullptr) return;

	m_deltaTime = GameTimer();
	m_deltaTime.Reset();

    MSG msg = {};

    while (m_isRunning)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
            {
                m_isRunning = false;
                break;
            }
        }

        if (!m_isRunning)
            break;

        //std::this_thread::sleep_for(std::chrono::milliseconds(5));

        m_deltaTime.Tick();
        InputSystem::HandleInputs();

        if (m_pAppManager)
        {
            m_pAppManager->Update(m_deltaTime);
        }

        if (m_pSceneManager)
        {
            m_pSceneManager->Update(m_deltaTime);

            if (m_pLightManager) {
                m_pLightManager->UpdateLightCB();
            }

            m_pSceneManager->Draw(m_deltaTime);
        }

        for (auto& [entity, sm] : m_stateMachineList)
        {
            if(sm->GetOwner()->IsActive())
            {
                sm->Update(m_deltaTime.DeltaTime());
            }
        }
    }
}

void GameManager::Destroy()
{
    delete s_pInstance;
    s_pInstance = nullptr;
}

Window& GameManager::GetWindow()
{
    return *m_pWindow;
}

RenderSystem& GameManager::GetRenderSystem()
{
	return *m_pRenderSystem;
}

AppManager& GameManager::GetAppManager()
{
    return *m_pAppManager;
}

EntityManager& GameManager::GetEntityManager()
{
    return *m_pEntityManager;
}

LightManager& GameManager::GetLightManager()
{
	return *m_pLightManager;
}

SceneManager& GameManager::GetSceneManager()
{
    return *m_pSceneManager;
}

StateMachine* GameManager::CreateStateMachine(Entity* owner)
{
    if (m_stateMachineList.contains(owner))
        return m_stateMachineList[owner].get();

    auto sm = std::make_unique<StateMachine>(owner);
    StateMachine* ptr = sm.get();

    m_stateMachineList[owner] = std::move(sm);

    return ptr;
}