#include "pch.h"
#include "FinalScene.h"

#include "CircuitGenerator.h"
#include "WaveManager.h"
#include "EnemyFactory.h"

#include "ScriptShoot.h"

FinalScene::~FinalScene()
{
}

void FinalScene::Init()
{
    m_pCamera = RenderSystem::Get().GetActiveCamera();
    if (!m_pCamera) return;

    gm = GameManager::GetInstance();
    sm = &gm->GetSceneManager();
    em = &gm->GetEntityManager();
    lm = &gm->GetLightManager();

    m_lastRailPos = XMFLOAT3();
    m_lastRailQuat = XMFLOAT4();

    entityCount = 0;

    m_playerHP = 50;

    Entity* plane = CreateEntity();
    plane->AddComponent<ComponentMeshRenderer>();
    plane->GetComponent<ComponentMeshRenderer>()
        ->SetGeometry(GeometryFactory::CreatePlane());
    XMFLOAT3 pos = { 0.0f, -2.f, 0.0f };
    plane->m_transform.SetPosition(pos);
    plane->m_transform.SetScale(200.f, 1.f, 200.f);
    plane->GetComponent<ComponentMeshRenderer>()->GetMaterial()->SetTexture("Floor");
    plane->GetComponent<ComponentMeshRenderer>()->GetMaterial()->SetColor(Color::LightBlack);

    if (em)
    {
        m_cameraLightEntity = em->CreateEntity();
        m_cameraLightEntity->SetName("Camera_Light");

        auto* camLightComp = m_cameraLightEntity->AddComponent<ComponentLight>();
        if (camLightComp)
        {
            camLightComp->SetType(LightType::POINT);
            camLightComp->SetColor(Color::GhostWhite);
            camLightComp->SetRange(65.f);
            camLightComp->SetIntensity(0.006f);
        }
        m_cameraLightEntity->SetActive(true);
    }

    m_cameraHeightOffset = 2.0f;
    m_cameraForwardOffset = -5.0f;
    m_cameraSideOffset = 0.0f;
    
	m_player = CreateEntity();
	m_player->SetName("Player");
	m_player->m_transform.SetPosition(XMFLOAT3(0.f, 0.f, 0.f));
	m_player->AddComponent<ComponentMeshRenderer>();
	m_player->GetComponent<ComponentMeshRenderer>()->SetGeometry(GeometryFactory::CreateSphere());
	m_player->GetComponent<ComponentMeshRenderer>()->GetMaterial()->SetColor(Color::Green);
	m_player->AddComponent<ComponentSphereCollider>();
	m_player->AddScript<ScriptShoot>();

    if (m_loadedRails.empty())
    {
        ImportExportJson jsonImporter;
        jsonImporter.LoadSceneFromJson(this, "../../saves/circuit.json");

        m_loadedRails.clear();
        for (Entity* e : GetEntitiesOfThisScene())
            if (e && e->GetName() == "Rails" && !e->HasParent())
                m_loadedRails.push_back(e);

        m_railIndexFloat = 0.f;
        m_followRails = false;

        Entity* firstRail = m_loadedRails[0];
        XMFLOAT3 pos = firstRail->m_transform.GetWorldPosition();
        pos.y += m_railOffsetY;

        m_followRails = true;
        m_railIndexFloat = 0.f;
        m_railSpeed = 2.0f;

        if (!m_followingWagon)
        {
            auto wagons = CreatePrefabs("Wagon", pos, m_pCamera->m_transform.GetQuat());
            if (!wagons.empty()) m_followingWagon = wagons[0];

            m_wagonLocalRotations.clear();
            std::function<void(Entity*)> SaveLocal;
            SaveLocal = [&](Entity* parent)
                {
                    if (!parent) return;
                    XMFLOAT4 parentQuat = parent->m_transform.GetQuat();
                    XMVECTOR parentQv = XMLoadFloat4(&parentQuat);

                    for (Entity* child : parent->GetChildren())
                    {
                        if (!child) continue;
                        XMFLOAT4 childQuat = child->m_transform.GetQuat();
                        XMVECTOR childQv = XMLoadFloat4(&childQuat);
                        XMVECTOR localQv = XMQuaternionNormalize(XMQuaternionMultiply(XMQuaternionInverse(parentQv), childQv));
                        XMFLOAT4 localQf; XMStoreFloat4(&localQf, localQv);
                        m_wagonLocalRotations[child] = localQf;

                        SaveLocal(child);
                    }
                };
            SaveLocal(m_followingWagon);

            XMFLOAT3 camPos = m_pCamera->m_transform.GetWorldPosition();
            XMFLOAT4 wagonQuat = m_followingWagon->m_transform.GetQuat();
            XMVECTOR invWagonQv = XMQuaternionInverse(XMLoadFloat4(&wagonQuat));
            XMFLOAT3 wagonPos = m_followingWagon->m_transform.GetWorldPosition();
            XMVECTOR localPosV = XMVector3Rotate(XMLoadFloat3(&camPos) - XMLoadFloat3(&wagonPos), invWagonQv);
            XMStoreFloat3(&m_cameraLocalOffsetPos, localPosV);

            m_cameraOffsetStored = true;
        }
    }

    CreateEnemies();

    CreateUI();
}

void FinalScene::Update(const GameTimer& gt)
{
    float dt = gt.DeltaTime();
    m_spawnRailTimer += dt;

    if (m_cameraLightEntity)
    {
        XMFLOAT3 camPos = m_pCamera->m_transform.GetWorldPosition();
        m_cameraLightEntity->m_transform.SetPosition(camPos);

        if (auto* lightComp = m_cameraLightEntity->GetComponent<ComponentLight>())
            lightComp->SetPosition(camPos.x, camPos.y, camPos.z);
    }

	m_player->m_transform.SetPosition(m_pCamera->m_transform.GetWorldPosition());
	m_player->m_transform.SetRotation(m_pCamera->m_transform.GetRotation());

    if (GetKeyDown(Keyboard::L))
        m_cameraLocked = !m_cameraLocked;

    m_score->GetComponent<ComponentText>()->SetText("Score : " + std::to_string(m_playerScore));
    m_HPTextEntity->GetComponent<ComponentText>()->SetText("Lives : " + std::to_string(m_playerHP));

    HandleRailFollow(dt);
    CreateRails(dt);
    UpdateRailDistance();
    UpdateEnemyDistance();
    IsPlayerAlive();
}

void FinalScene::UpdateRailDistance()
{
    if (!m_pCamera || m_loadedRails.empty()) return;

    XMFLOAT3 camPos = m_pCamera->m_transform.GetWorldPosition();
    std::function<void(Entity*, float)> ApplyOverrideRecursively;
    ApplyOverrideRecursively = [&](Entity* e, float weight)
        {
            if (!e) return;
            auto* mesh = e->GetComponent<ComponentMeshRenderer>();
            if (mesh)
            {
                DirectX::XMFLOAT4 overrideColor = Color::NormalizeColor(Color::Pink);
                overrideColor.w = weight;
                mesh->SetOverrideColor(overrideColor);
            }
            for (auto* child : e->GetChildren())
                ApplyOverrideRecursively(child, weight);
        };

    const float fadeRange = 100.0f;
    for (auto* rail : m_loadedRails)
    {
        if (!rail) continue;
        XMFLOAT3 railPos = rail->m_transform.GetWorldPosition();
        float dx = camPos.x - railPos.x;
        float dy = camPos.y - railPos.y;
        float dz = camPos.z - railPos.z;
        float dist = sqrtf(dx * dx + dy * dy + dz * dz);
        float t = std::clamp(dist / fadeRange, 0.0f, 1.0f);
        float weight = 1.0f - t;
        ApplyOverrideRecursively(rail, weight);
    }
}

void FinalScene::UpdateEnemyDistance()
{
    if (!m_pCamera) return;

    const float fadeStart = 10.0f; 
    const float fadeEnd   = 80.0f; 
    const float invRange  = 1.0f / (fadeEnd - fadeStart);

    XMFLOAT3 camPos = m_pCamera->m_transform.GetWorldPosition();

    static std::unordered_map<Entity*, DirectX::XMFLOAT4> baseColorByEntity;

    auto smootherstep = [](float x) -> float {

        return x * x * x * (x * (x * 6.0f - 15.0f) + 10.0f);
    };

    for (Entity* e : GetEntitiesOfThisScene())
    {
        if (!e) continue;

        const std::string& name = e->GetName();
        if (name.find("Enemy") == std::string::npos)
            continue;

        auto* mesh = e->GetComponent<ComponentMeshRenderer>();
        if (!mesh) continue;

        if (baseColorByEntity.find(e) == baseColorByEntity.end())
        {
            baseColorByEntity[e] = mesh->GetOverrideColor();
        }
        DirectX::XMFLOAT4 base = baseColorByEntity[e];

        XMFLOAT3 p = e->m_transform.GetWorldPosition();
        float dx = camPos.x - p.x;
        float dy = camPos.y - p.y;
        float dz = camPos.z - p.z;
        float dist = sqrtf(dx * dx + dy * dy + dz * dz);

        float t = 0.0f;
        if (dist <= fadeStart) t = 0.0f;
        else if (dist >= fadeEnd) t = 1.0f;
        else t = (dist - fadeStart) * invRange;

        float s = smootherstep(t);

        float weight = 1.0f - s;

        DirectX::XMFLOAT4 newColor;
        newColor.x = base.x * weight;
        newColor.y = base.y * weight;
        newColor.z = base.z * weight;
        newColor.w = weight;

        mesh->SetOverrideColor(newColor);
    }
}

void FinalScene::HandleRailFollow(float dt)
{
    /*if (GetKeyUp(Keyboard::U))
    {
        ImportExportJson jsonImporter;
        jsonImporter.LoadSceneFromJson(this, "../../saves/circuit.json");

        m_loadedRails.clear();
        for (Entity* e : GetEntitiesOfThisScene())
            if (e && e->GetName() == "Rails" && !e->HasParent())
                m_loadedRails.push_back(e);

        m_railIndexFloat = 0.f;
        m_followRails = false;
    }

    if (GetKeyUp(Keyboard::Y))
    {
        ImportExportJson jsonExporter;
        jsonExporter.SaveSceneToJson(this, "../../saves/circuit.json");
    }

    if (GetKeyDown(Keyboard::P) && !m_loadedRails.empty())
    {
        Entity* firstRail = m_loadedRails[0];
        XMFLOAT3 pos = firstRail->m_transform.GetWorldPosition();
        pos.y += m_railOffsetY;

        m_followRails = true;
        m_railIndexFloat = 0.f;
        m_railSpeed = 2.0f;

        if (!m_followingWagon)
        {
            auto wagons = CreatePrefabs("Wagon", pos, m_pCamera->m_transform.GetQuat());
            if (!wagons.empty()) m_followingWagon = wagons[0];

            m_wagonLocalRotations.clear();
            std::function<void(Entity*)> SaveLocal;
            SaveLocal = [&](Entity* parent)
                {
                    if (!parent) return;
                    XMFLOAT4 parentQuat = parent->m_transform.GetQuat();
                    XMVECTOR parentQv = XMLoadFloat4(&parentQuat);

                    for (Entity* child : parent->GetChildren())
                    {
                        if (!child) continue;
                        XMFLOAT4 childQuat = child->m_transform.GetQuat();
                        XMVECTOR childQv = XMLoadFloat4(&childQuat);
                        XMVECTOR localQv = XMQuaternionNormalize(XMQuaternionMultiply(XMQuaternionInverse(parentQv), childQv));
                        XMFLOAT4 localQf; XMStoreFloat4(&localQf, localQv);
                        m_wagonLocalRotations[child] = localQf;

                        SaveLocal(child);
                    }
                };
            SaveLocal(m_followingWagon);

            XMFLOAT3 camPos = m_pCamera->m_transform.GetWorldPosition();
            XMFLOAT4 wagonQuat = m_followingWagon->m_transform.GetQuat();
            XMVECTOR invWagonQv = XMQuaternionInverse(XMLoadFloat4(&wagonQuat));
            XMFLOAT3 wagonPos = m_followingWagon->m_transform.GetWorldPosition();
            XMVECTOR localPosV = XMVector3Rotate(XMLoadFloat3(&camPos) - XMLoadFloat3(&wagonPos), invWagonQv);
            XMStoreFloat3(&m_cameraLocalOffsetPos, localPosV);

            m_cameraOffsetStored = true;
        }
    }*/

    if (GetKey(Keyboard::UP)) m_railSpeed += 5.f * dt;
    if (GetKey(Keyboard::DOWN)) m_railSpeed -= 5.f * dt;
    m_railSpeed = std::clamp(m_railSpeed, 0.f, 20.f);

    //if (GetKeyDown(Keyboard::T) && m_hasLastRail)
    //{
    //    XMVECTOR forward = XMVector3Rotate(XMVectorSet(0, 0, 1, 0), XMLoadFloat4(&m_lastRailQuat));
    //    XMVECTOR newPos = XMLoadFloat3(&m_lastRailPos) + forward * 5.0f;
    //    XMFLOAT3 out; XMStoreFloat3(&out, newPos);
    //    m_pCamera->m_transform.SetPosition(out);
    //    m_pCamera->m_transform.SetQuat(m_lastRailQuat);
    //}

    //if (GetKeyDown(Keyboard::R) && m_hasLastRail)
    //{
    //    m_pCamera->m_transform.SetPosition(m_lastRailPos);
    //    m_pCamera->m_transform.SetQuat(m_lastRailQuat);
    //}

    if (m_followRails && !m_loadedRails.empty() && m_followingWagon)
    {
        m_railIndexFloat += m_railSpeed * dt;
        if (m_railIndexFloat >= static_cast<float>(m_loadedRails.size()))
            m_railIndexFloat = 0.f;

        int index0 = static_cast<int>(floor(m_railIndexFloat));
        int index1 = (index0 + 1) % static_cast<int>(m_loadedRails.size());
        float t = m_railIndexFloat - index0;

        XMFLOAT3 pos0 = m_loadedRails[index0]->m_transform.GetWorldPosition();
        XMFLOAT3 pos1 = m_loadedRails[index1]->m_transform.GetWorldPosition();
        XMFLOAT3 railPos = { pos0.x + (pos1.x - pos0.x) * t,
                             pos0.y + (pos1.y - pos0.y) * t,
                             pos0.z + (pos1.z - pos0.z) * t };

        XMFLOAT4 quat0 = m_loadedRails[index0]->m_transform.GetQuat();
        XMFLOAT4 quat1 = m_loadedRails[index1]->m_transform.GetQuat();
        XMVECTOR q0 = XMLoadFloat4(&quat0);
        XMVECTOR q1 = XMLoadFloat4(&quat1);
        XMVECTOR qInterp = XMQuaternionSlerp(q0, q1, t);

        XMFLOAT3 wagonPos = railPos; wagonPos.y += 2.f;
        m_followingWagon->m_transform.SetPosition(wagonPos);
        XMFLOAT4 wagonQuat; XMStoreFloat4(&wagonQuat, qInterp);
        m_followingWagon->m_transform.SetQuat(wagonQuat);

        std::function<void(Entity*, const XMFLOAT3&, const XMFLOAT4&)> Propagate;
        Propagate = [&](Entity* parent, const XMFLOAT3& parentPos, const XMFLOAT4& parentQuat)
            {
                if (!parent) return;
                XMVECTOR parentQv = XMLoadFloat4(&parentQuat);
                for (Entity* child : parent->GetChildren())
                {
                    if (!child) continue;
                    child->m_transform.SetPosition(parentPos);

                    XMVECTOR localQv = XMQuaternionIdentity();
                    auto it = m_wagonLocalRotations.find(child);
                    if (it != m_wagonLocalRotations.end())
                        localQv = XMLoadFloat4(&it->second);
                    else
                        localQv = XMLoadFloat4(&child->m_transform.LocalRotation());

                    XMVECTOR childWorldQv = XMQuaternionNormalize(XMQuaternionMultiply(parentQv, localQv));
                    XMFLOAT4 childWorldQf; XMStoreFloat4(&childWorldQf, childWorldQv);
                    child->m_transform.SetQuat(childWorldQf);

                    Propagate(child, parentPos, childWorldQf);
                }
            };
        Propagate(m_followingWagon, wagonPos, wagonQuat);

        XMVECTOR up = XMVector3Rotate(XMVectorSet(0, 2.5, 2.8, 0), qInterp);
        XMVECTOR forward = XMVector3Rotate(XMVectorSet(0, 0, 1, 0), qInterp);
        XMVECTOR side = XMVector3Rotate(XMVectorSet(1, 0, 0, 0), qInterp);
        XMVECTOR finalPos = XMLoadFloat3(&railPos) + up * m_cameraHeightOffset + forward * m_cameraForwardOffset + side * m_cameraSideOffset;

        XMFLOAT3 camPos; XMStoreFloat3(&camPos, finalPos);
        m_pCamera->m_transform.SetPosition(camPos);

        if (!m_cameraLocked)
        {
            XMFLOAT4 camQuat = m_pCamera->m_transform.GetQuat();
            m_pCamera->m_transform.SetQuat(camQuat);
        }
    }
}

void FinalScene::CreateRails(float dt)
{
    //if (GetKey(Keyboard::B) && m_spawnRailTimer >= m_spawnRailInterval)
    //{
    //    XMFLOAT3 pos = m_pCamera->m_transform.GetWorldPosition();
    //    XMFLOAT4 quat = m_pCamera->m_transform.GetQuat();
    //    CreatePrefabs("Rails", pos, quat);
    //    m_lastRailPos = pos; m_lastRailQuat = quat; m_hasLastRail = true;
    //    m_spawnRailTimer = 0;
    //}

    //if (GetKeyDown(Keyboard::T) && m_hasLastRail)
    //{
    //    XMVECTOR forward = XMVector3Rotate(XMVectorSet(0, 0, 1, 0), XMLoadFloat4(&m_lastRailQuat));
    //    XMVECTOR newPos = XMLoadFloat3(&m_lastRailPos) + forward * 5.0f;
    //    XMFLOAT3 out; XMStoreFloat3(&out, newPos);
    //    m_pCamera->m_transform.SetPosition(out);
    //    m_pCamera->m_transform.SetQuat(m_lastRailQuat);
    //}

    //if (GetKeyDown(Keyboard::R) && m_hasLastRail)
    //{
    //    m_pCamera->m_transform.SetPosition(m_lastRailPos);
    //    m_pCamera->m_transform.SetQuat(m_lastRailQuat);
    //}

    //if (GetKey(Keyboard::V) && m_spawnRailTimer >= m_spawnRailInterval)
    //{
    //    XMFLOAT3 pos = m_pCamera->m_transform.GetWorldPosition();
    //    XMFLOAT4 quat = m_pCamera->m_transform.GetQuat();
    //    CreatePrefabs("StreetLight", pos, quat);
    //    m_spawnRailTimer = 0;
    //}

    //if (GetKeyDown(Keyboard::W) && !m_loadedRails.empty())
    //{
    //    Entity* firstRail = m_loadedRails[0];
    //    XMFLOAT3 pos = firstRail->m_transform.GetWorldPosition();
    //    XMFLOAT4 quat = firstRail->m_transform.GetQuat();
    //    pos.y += m_railOffsetY - 1.5f;
    //    CreatePrefabs("Wagon", pos, quat);
    //}
}

void FinalScene::Draw(const GameTimer& gt) {}

void FinalScene::CreateEnemies()
{
    Entity* waveManager = CreateEntity();
    waveManager->AddScript<WaveManagerScript>()->SetLoadedRails(m_loadedRails);
}

void FinalScene::CreateUI()
{
    {
        m_score = CreateEntity();
        m_score->m_transform.SetPosition(XMFLOAT3(500.f, 10.f, 0.f));

        auto text = m_score->AddComponent<ComponentText>();
        text->SetFont("Pixel");
        text->SetText("Score : " + std::to_string(m_playerScore));
        text->SetScale(XMFLOAT2(20, 25));
        {
            XMFLOAT3 p = m_score->m_transform.GetLocalPosition();
            p.z = 0.0f;
            m_score->m_transform.SetPosition(p);
        }
        m_score->SetName("ScoreText");
    }

    {
        m_HPTextEntity = CreateEntity();
        m_HPTextEntity->m_transform.SetPosition(XMFLOAT3(125.f, 10.f, 0.f));

        auto text = m_HPTextEntity->AddComponent<ComponentText>();
        text->SetFont("Pixel");
        text->SetText("Lives : " + std::to_string(m_playerHP));
        text->SetScale(XMFLOAT2(20, 25));
        {
            XMFLOAT3 p = m_HPTextEntity->m_transform.GetLocalPosition();
            p.z = 0.0f;
            m_HPTextEntity->m_transform.SetPosition(p);
        }
        m_HPTextEntity->SetName("HPText");
    }
}

int FinalScene::GetPlayerScore()
{
    return m_playerScore;
}

void FinalScene::SetPlayerScore(int value)
{
    m_playerScore = value;
}

void FinalScene::AddToPlayerScore(int value)
{
    m_playerScore += value;
}

int FinalScene::GetPlayerHP()
{
    return m_playerHP;
}

void FinalScene::SetPlayerHP(int value)
{
    m_playerHP = value;
}

void FinalScene::AddToPlayerHP(int value)
{
    m_playerHP += value;
}

void FinalScene::IsPlayerAlive()
{
    if (m_playerHP <= 0)
    {
        EndTheGame();
    }
}

void FinalScene::EndTheGame()
{
    PostQuitMessage(0);
}