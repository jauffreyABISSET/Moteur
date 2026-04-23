#include "pch.h"
#include "SceneGamePlay.h"
#include "Engine/InputsEnums.h"
#include "GamePlayApp.h"
#include <RandomScene.h>
#include "CircuitGenerator.h"


GamePlayScene::~GamePlayScene()
{

}

void GamePlayScene::Init()
{
    gm = GameManager::GetInstance();
    sm = &gm->GetSceneManager();
    em = &gm->GetEntityManager();
    lm = &gm->GetLightManager();

    RandomTestEntity();
}

void GamePlayScene::Update(const GameTimer& gt)
{
    GameManager* gm = GameManager::GetInstance();
    EntityManager& em = gm->GetEntityManager();
    SceneManager& sm = gm->GetSceneManager();

    if (test) {

        //CreatePrefabs("StreetLamp", XMFLOAT3(5, 0, 5));
        CreatePrefabs("StreetSpot", XMFLOAT3(0, 5, -5));
        test = false;
    }
    if (GetKeyDown(Keyboard::CONTROL))
        sm.SwitchScene<RandomScene>();

    if (GetKeyDown(Keyboard::C))
        sm.SwitchScene<CircuitGenerator>();
}

void GamePlayScene::Draw(const GameTimer& gt)
{

}

void GamePlayScene::RandomTestEntity()
{
    Entity* cube = CreateEntity();

    cube->AddComponent<ComponentMeshRenderer>();
    cube->GetComponent<ComponentMeshRenderer>()->SetGeometry(GeometryFactory::CreateCube());
    cube->GetComponent<ComponentMeshRenderer>()->GetMaterial()->SetTexture("turtle");
    cube->SetName("CubeTestTexture");
    cube->m_transform.SetRotationDegrees(XMFLOAT3(0.f, 0.f, 180.f));

    StateMachine* sm = GameManager::GetInstance()->CreateStateMachine(cube);

    sm->AddState("Idle",
        [](Entity* e, float dt)
        {
            std::cout << "Enter Idle\n";
        },
        [](Entity* e, float dt)
        {
            e->m_transform.Rotate(XMFLOAT3(0.f, XMConvertToRadians(45.f) * 0.016f, 0.f));
        },
        [](Entity* e, float dt)
        {
            std::cout << "Exit Idle\n";
        }
    );

    sm->AddState("Test",
        [](Entity* e, float dt)
        {
            std::cout << "Enter Test\n";
        },
        [](Entity* e, float dt)
        {
            e->m_transform.Rotate(XMFLOAT3(0.f, XMConvertToRadians(360.f) * 0.016f, 0.f));
        },
        [](Entity* e, float dt)
        {
            std::cout << "Exit Test\n";
        }
    );

    sm->AddTransition(
        "Idle",
        "Test",
        {
            [sm](Entity*, float dt)
            {
                return sm->GetStateTime() > 2.0f;
            }
        }
    );

    sm->AddTransition(
        "Test",
        "Idle",
        {
            [sm](Entity*, float dt)
            {
                return sm->GetStateTime() > 2.0f;
            }
        }
    );

    sm->SetInitialState("Idle");

    Entity* pyramid = CreateEntity();
    pyramid->AddComponent<ComponentMeshRenderer>();
    pyramid->GetComponent<ComponentMeshRenderer>()->SetGeometry(GeometryFactory::CreatePyramid());
    pyramid->GetComponent<ComponentMeshRenderer>()->GetMaterial()->SetColor(Color::White);
    pyramid->GetComponent<ComponentMeshRenderer>()->GetMaterial()->SetTexture("turtule");
    pyramid->SetName("PyramidTest");

    Entity* e = CreateEntity();
    e->AddComponent<ComponentMeshRenderer>();
    e->GetComponent<ComponentMeshRenderer>()->SetGeometry(GeometryFactory::CreateCylinder());
    e->GetComponent<ComponentMeshRenderer>()->GetMaterial()->SetColor(Color::Cyan);
    e->SetName("Test1");

    Entity* e2 = CreateEntity();
    e2->AddComponent<ComponentMeshRenderer>();
    e2->GetComponent<ComponentMeshRenderer>()->SetGeometry(GeometryFactory::CreateCylinder());
    e2->SetName("Test2");

    Entity* e4 = CreateEntity();

    e4->m_transform.SetScale(XMFLOAT3(10.f, 0.01f, 10.f));

    e4->AddComponent<ComponentMeshRenderer>();
    e4->GetComponent<ComponentMeshRenderer>()->SetGeometry(GeometryFactory::CreatePlane());
    e4->AddComponent<ComponentBoxCollider>();
    e4->AddComponent<ComponentCollider>();
    e4->AddComponent<ComponentRigidBody>();
    e4->GetComponent<ComponentRigidBody>()->SetKinematic(true);
    e4->m_transform.SetScale(XMFLOAT3(10.f, 0.01f, 10.f));

    e4->SetName("Ground");

    Entity* e5 = CreateEntity();

    e5->m_transform.SetScale(XMFLOAT3(10.f, 0.01f, 10.f));
    e5->m_transform.SetRotationDegrees(90.f, 0.f, 0.f);

    e5->AddComponent<ComponentMeshRenderer>();
    e5->GetComponent<ComponentMeshRenderer>()->SetGeometry(GeometryFactory::CreatePlane());
    e5->AddComponent<ComponentBoxCollider>();
    e5->GetComponent<ComponentBoxCollider>()->SetRotation(XMFLOAT3(90.f, 0.f, 0.f));
    e5->AddComponent<ComponentCollider>();
    e5->AddComponent<ComponentRigidBody>();
    e5->GetComponent<ComponentRigidBody>()->SetKinematic(true);
    e5->GetComponent<ComponentMeshRenderer>()->GetMaterial()->SetTexture("turtule");

    e5->SetName("Wall");

    Entity* e7 = CreateEntity();

    e7->m_transform.SetScale(XMFLOAT3(10.f, 0.01f, 10.f));
    e7->m_transform.SetRotationDegrees(90.f, 0.f, 0.f);

    e7->AddComponent<ComponentMeshRenderer>();
    e7->GetComponent<ComponentMeshRenderer>()->SetGeometry(GeometryFactory::CreatePlane());
    e7->AddComponent<ComponentBoxCollider>();
    e7->GetComponent<ComponentBoxCollider>()->SetRotation(XMFLOAT3(90.f, 0.f, 0.f));
    e7->AddComponent<ComponentCollider>();
    e7->AddComponent<ComponentRigidBody>();
    e7->GetComponent<ComponentRigidBody>()->SetKinematic(true);

    e7->SetName("testmur2");

    cube->m_transform.SetPosition(XMFLOAT3(0.f, 5.f, 0.f));
    e->m_transform.SetPosition(XMFLOAT3(5.f, 10.f, 0.f));
    pyramid->m_transform.SetPosition(XMFLOAT3(0.f, 10.f, 0.f));
    e2->m_transform.SetPosition(XMFLOAT3(-10.f, 0.f, 10.f));
    e4->m_transform.SetPosition(XMFLOAT3(0.f, -10.f, 0.f));
    e5->m_transform.SetPosition(XMFLOAT3(0.f, 0.f, 10.f));
    e7->m_transform.SetPosition(XMFLOAT3(0.f, 0.f, -30.f));

    //test ui text
    Entity* score = CreateEntity();
    score->m_transform.SetPosition(XMFLOAT3(2.f, 2.f, 0.f));

    auto text = score->AddComponent<ComponentText>();
    text->SetFont("Pixel");
    text->SetText("Hugo est le plus Beau !!!");
    text->SetScale(XMFLOAT2(10,15));

    {
        XMFLOAT3 p = score->m_transform.GetLocalPosition();
        p.z = 0.0f;
        score->m_transform.SetPosition(p);
    }

    score->SetName("ScoreText");

    Camera* p_cam = RenderSystem::Get().GetActiveCamera();
    p_cam->LookAt(cube->m_transform.GetWorldPosition());
}

void ScriptTest::Init()
{

}

void ScriptTest::Update(float dt)
{
    float rotCoeff = 0.1f;

    const float periodSeconds = 2.0f;
    const float degPerSec = 360.0f / periodSeconds;
    float deltaRad = XMConvertToRadians(degPerSec * dt);

    m_pEntity->m_transform.Rotate(XMFLOAT3{ deltaRad, deltaRad, 0.0f });

    XMFLOAT3 center(0.f, 0.f, 0.f);
    XMFLOAT3 axis(0.f, 1.f, 0.f);
    float angularSpeed = rotCoeff * XMConvertToRadians(degPerSec);

    m_pEntity->m_transform.WorldOscillation(XMFLOAT3(0.f, 1.f, 0.f), 5.5f, 1.f, dt);
    m_pEntity->m_transform.OrbitAroundAxis(center, axis, 50.f, angularSpeed, dt);
}

std::unique_ptr<Script> ScriptTest::Clone() const
{
    return std::make_unique<ScriptTest>(*this);
}