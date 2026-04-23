#include "pch.h"
#include "SceneManager.h"

SceneManager::SceneManager()
{
    m_currentScene = 0;
    m_previousScene = -1;
}

SceneManager::~SceneManager()
{
    for (Scene* s : m_scenes) {
        delete s;
    }
    m_scenes.clear();
    m_sceneInitialized.clear();
    m_sceneActive.clear();
}

void SceneManager::Init()
{
    if (m_currentScene < 0 || static_cast<size_t>(m_currentScene) >= m_scenes.size())
        return;

    if (m_sceneInitialized[m_currentScene] == false)
    {
        if (m_scenes[m_currentScene])
            m_scenes[m_currentScene]->Init();

        m_sceneInitialized[m_currentScene] = true;
        m_sceneActive[m_currentScene] = true;

        auto entities = m_scenes[m_currentScene]->GetEntitiesOfThisScene();
        for (Entity* e : entities)
            if (e) {
                e->InitComponents();
                e->InitScript();
            }
    }
}

void SceneManager::Update(const GameTimer& gt)
{
    if (m_sceneInitialized[m_currentScene] == false)
        Init();

    float dt = gt.DeltaTime();

    for (size_t i = 0; i < m_scenes.size(); ++i)
    {
        if (!m_sceneActive[i]) continue;

        auto entities = m_scenes[i]->GetEntitiesOfThisScene();

        // Delete entities with m_destroyed tag
        //auto it = entities.begin();
        //while (it != entities.end())
        //{
        //    Entity* e = *it;
        //    if (e && e->IsDestroyed())
        //    {
        //        delete e;
        //        it = entities.erase(it);
        //    }
        //    else
        //    {
        //        ++it;
        //    }
        //}

        // Updtae entities
        for (Entity* e : entities)
        {
            if (!e || !e->IsActive()) continue;

            if (e->HasParent())
            {
                XMFLOAT3 parentWorldPos = e->GetParent()->m_transform.GetWorldPosition();
                XMFLOAT4 parentWorldQuat = e->GetParent()->m_transform.GetQuat();

                e->m_transform.SetPosition(e->m_transform.GetLastWorldPosition());
                e->m_transform.SetQuat(e->m_transform.GetLocalRotationQuaternion());

                continue;
            }

            e->UpdateComponents(dt);
            e->UpdateScripts(dt);
        }

        // Updtae the scene
        if (m_sceneActive[i] && m_scenes[i])
            m_scenes[i]->Update(gt);
    }
}

void SceneManager::Draw(const GameTimer& gt)
{
    if (m_sceneInitialized[m_currentScene] == false)
        Init();

    for (int activeScene = 0; activeScene < static_cast<int>(m_scenes.size()); ++activeScene)
        if (m_sceneActive[activeScene])
        {
            RenderSystem::Get().DrawCommon();
            m_scenes[activeScene]->Draw(gt);
            RenderSystem::Get().EndCommonDraw();
        }
}

void SceneManager::SwitchScene(int sceneId, bool init, bool stayActive)
{
    if (sceneId < 0 || sceneId >= static_cast<int>(m_scenes.size()))
        return;

    if (!stayActive)
        SetSceneActive(m_currentScene, false);

    m_previousScene = m_currentScene;
    m_currentScene = sceneId;

    if (init)
    {
        if (!m_sceneInitialized[static_cast<size_t>(m_currentScene)])
        {
            if (m_scenes[m_currentScene])
            {
                m_scenes[m_currentScene]->Init();

                auto entities = m_scenes[m_currentScene]->GetEntitiesOfThisScene();
                for (Entity* e : entities)
                    if (e) e->InitComponents();

                m_sceneInitialized[static_cast<size_t>(m_currentScene)] = true;
            }
        }
    }
    SetSceneActive(m_currentScene, true);
}

void SceneManager::SetSceneActive(int sceneId, bool active)
{
    if (sceneId < 0 || sceneId >= static_cast<int>(m_scenes.size()))
        return;

    m_sceneActive[sceneId] = active;

    Scene* s = m_scenes[sceneId];
    if (s) {
        switch (active) {
        case true:
            s->SetStateEntities(true);
            break;
        case false:
            s->SetStateEntities(false);
            break;
        }
    }
}

void SceneManager::SetSceneActiveAll(bool active)
{
    for (size_t i = 0; i < m_sceneActive.size(); ++i)
        m_sceneActive[i] = active;
}

void SceneManager::SetSceneActiveCurrent(bool active)
{
    if (m_currentScene < 0 || static_cast<size_t>(m_currentScene) >= m_scenes.size())
        return;

    if (m_sceneInitialized.size() <= static_cast<size_t>(m_currentScene))
        return;

    m_sceneActive[m_currentScene] = active;
}

Scene* SceneManager::GetCurrentScene() const
{
    return (m_currentScene >= 0 && m_currentScene < static_cast<int>(m_scenes.size())) ? m_scenes[m_currentScene] : nullptr;
}

std::vector<Scene*> SceneManager::GetScenes() const
{
    return m_scenes;
}

std::vector<Scene*> SceneManager::GetActiveScenes() const
{
    std::vector<Scene*> activeScenes;

    for (size_t i = 0; i < m_scenes.size(); ++i)
        if (m_sceneActive[i])
            activeScenes.push_back(m_scenes[i]);
    return activeScenes;
}
