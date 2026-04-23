#include "pch.h"
#include "CircuitGenerator.h"
#include "SceneGamePlay.h"

void CircuitGenerator::Init()
{
    m_pCamera = RenderSystem::Get().GetActiveCamera();

	Entity* e = CreateEntity();
	e->AddComponent<ComponentMeshRenderer>();
	e->GetComponent<ComponentMeshRenderer>()->SetGeometry(GeometryFactory::CreateSphere());
}

void CircuitGenerator::Update(const GameTimer& gt)
{
    float dt = gt.DeltaTime();

    m_spawnRailTimer += dt;

    InputScene(dt);
}

void CircuitGenerator::Draw(const GameTimer& gt)
{

}

void CircuitGenerator::InputScene(float dt)
{

    if (GetKey(Keyboard::B)) {
        if (m_spawnRailTimer >= m_spawnRailInterval) {

            XMFLOAT3 pos = m_pCamera->m_transform.GetWorldPosition();
            XMFLOAT4 quat = m_pCamera->m_transform.GetQuat();

            CreatePrefabs("Rails", pos, quat);

            m_lastRailPos = pos;
            m_lastRailQuat = quat;
            m_hasLastRail = true;

            m_spawnRailTimer = 0;
        }
    }

    if (GetKeyDown(Keyboard::T))
    {
        if (m_hasLastRail)
        {
            XMVECTOR forward = XMVector3Rotate(
                XMVectorSet(0, 0, 1, 0),
                XMLoadFloat4(&m_lastRailQuat)
            );

            XMVECTOR basePos = XMLoadFloat3(&m_lastRailPos);
            XMVECTOR newPos = basePos + forward * 5.0f;

            XMFLOAT3 out;
            XMStoreFloat3(&out, newPos);

            m_pCamera->m_transform.SetPosition(out);
            m_pCamera->m_transform.SetQuat(m_lastRailQuat);
        }
    }

    if (GetKeyDown(Keyboard::R))
    {
        if (m_hasLastRail)
        {
            m_pCamera->m_transform.SetPosition(m_lastRailPos);
            m_pCamera->m_transform.SetQuat(m_lastRailQuat);
        }
    }

    if (GetKey(Keyboard::V)) {
        if (m_spawnRailTimer >= m_spawnRailInterval) {
            XMFLOAT3 pos = m_pCamera->m_transform.GetWorldPosition();
			XMFLOAT4 quat = m_pCamera->m_transform.GetQuat();

            CreatePrefabs("StreetLamp", pos, quat);
            m_spawnRailTimer = 0;
        }
    }

    if (GetKey(Keyboard::W)) {
        if (m_spawnRailTimer >= m_spawnRailInterval) {
            XMFLOAT3 pos = m_pCamera->m_transform.GetWorldPosition();
            XMFLOAT4 rot = m_pCamera->m_transform.GetLocalRotationQuaternion();
			pos.y -= 1.f;
            m_instance1 = CreatePrefabs("Wagon", pos);
            m_spawnRailTimer = 0;
        }
    }

    if (GetKeyDown(Keyboard::SHIFT)) {
        GameManager* gm = GameManager::GetInstance();
        SceneManager& sm = gm->GetSceneManager();

        sm.SwitchScene<GamePlayScene>();
    }

    if (GetKeyUp(Keyboard::U))
	{
        ImportExportJson jsonImporter;
        jsonImporter.LoadSceneFromJson(this, "../../saves/circuit.json");
	}

    if (GetKeyUp(Keyboard::Y))
    {
		ImportExportJson jsonExporter;
        jsonExporter.SaveSceneToJson(this, "../../saves/circuit.json");
    }
}

