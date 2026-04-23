#include "pch.h"
#include "RandomScene.h"
#include "SceneGamePlay.h"

RandomScene::~RandomScene()
{

}

void RandomScene::Init()
{
	Entity* e = CreateEntity();
    e->AddComponent<ComponentMeshRenderer>();
	e->GetComponent<ComponentMeshRenderer>()->SetGeometry(GeometryFactory::CreateCube());
	e->AddComponent<ComponentCollider>();
    e->AddComponent<ComponentGravity>();
    e->AddComponent<ComponentBoxCollider>();

    Entity* e2 = CreateEntity();
    e2->AddComponent<ComponentMeshRenderer>();
    e2->GetComponent<ComponentMeshRenderer>()->SetGeometry(GeometryFactory::CreateSphere());
    e2->AddComponent<ComponentSphereCollider>();
    e2->AddComponent<ComponentCollider>();
    e2->AddComponent<ComponentGravity>();

    e2->m_transform.SetPosition(XMFLOAT3(10.f, 23.f, -10.f));
    e2->m_transform.SetScale(XMFLOAT3(5.f, 5.f, 5.f));

    Entity* e3 = CreateEntity();
    e3->AddComponent<ComponentMeshRenderer>();
    e3->GetComponent<ComponentMeshRenderer>()->SetGeometry(GeometryFactory::CreateSphere());
    e3->AddComponent<ComponentSphereCollider>();
    e3->AddComponent<ComponentCollider>();
    e3->AddComponent<ComponentGravity>();

    e3->m_transform.SetPosition(XMFLOAT3(11.f, 10.f, -11.f));
	e3->m_transform.SetScale(XMFLOAT3(5.f, 5.f, 5.f));

    e3->SetName("Ball");

    Entity* e7 = CreateEntity();
    e7->AddComponent<ComponentMeshRenderer>();
    e7->GetComponent<ComponentMeshRenderer>()->SetGeometry(GeometryFactory::CreateSphere());
    e7->AddComponent<ComponentSphereCollider>();
    e7->AddComponent<ComponentCollider>();

    e7->m_transform.SetPosition(XMFLOAT3(0.f, 5.f, -10.f));

	e7->SetName("missile");

}

void RandomScene::Update(const GameTimer& gt)
{
	float dt = gt.DeltaTime();
    for (Entity* e : GetEntitiesOfThisScene())
    {
        if (!e->IsActive())
            continue;
		if (e->GetName() == "missile" && m_test)
			e->m_transform.GoForward(10.f, dt);
        else if (e->GetName() == "missile")
            e->m_transform.GoBackward(10, dt);
    }

    if (GetKeyDown(Keyboard::CONTROL)) {
        GameManager* gm = GameManager::GetInstance();
        SceneManager& sm = gm->GetSceneManager();

        sm.SwitchScene<GamePlayScene>();
    }

    if (GetKeyUp(Keyboard::R)) {
		m_test = !m_test;
	}

}

void RandomScene::Draw(const GameTimer& gt)
{

}
