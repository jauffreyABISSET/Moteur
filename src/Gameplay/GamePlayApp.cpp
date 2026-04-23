#include "pch.h"
#include "GamePlayApp.h"

#include "SceneGamePlay.h"
#include "RandomScene.h"
#include "CircuitGenerator.h"
#include "FinalScene.h"

GamePlayApp::GamePlayApp(Window& win) : m_win(win)
{

}

void GamePlayApp::Init()
{
    GameManager* gm = GameManager::GetInstance();
    m_sm = &gm->GetSceneManager();
    m_em = &gm->GetEntityManager();
    m_window = &gm->GetWindow();


    m_sm->AddScene<GamePlayScene>();
    m_sm->AddScene<RandomScene>();
    m_sm->AddScene<CircuitGenerator>();
    m_sm->AddScene<FinalScene>();

    TextureRegister();
    FontRegister();
	CreatePrefabs();


    m_pCamera = new Camera();

    XMFLOAT3 camPos = { 0.0f, 0.0f, -5.0f };
    m_pCamera->m_transform.SetPosition(camPos);
    m_pCamera->SetLens(0.25f * XM_PI, m_win.AspectRatio(), 0.1f, 500.0f);

    m_pCamera->LookAt(XMLoadFloat3(&camPos), XMVectorZero(), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
    m_pCamera->UpdateViewMatrix();

    RenderSystem::Get().SetActiveCamera(m_pCamera);

    RenderSystem::Get().InitializeDeviceResources(8192);

    m_sm->SwitchScene<FinalScene>();
}

void GamePlayApp::Update(const GameTimer& gt)
{
	m_pCamera = RenderSystem::Get().GetActiveCamera();

    if ( m_pCamera == nullptr )
		return;

    float dt = gt.DeltaTime();

    mLastMousePos = GetMousePosition();

    XMFLOAT2 mouse = GetMousePosition();
    float dx = mouse.x - mLastMousePos.x;
    float dy = mouse.y - mLastMousePos.y;

    float speed = 5.0f;
    float mouseSens = 0.3f;
    float rollSpeed = 40.0f;

 //   if (GetKeyDown(Keyboard::E)) {
	//	speed = 20.0f;
	//}

    if (GetKey(Keyboard::Z))
        m_pCamera->m_transform.GoForward(speed, dt);
    if (GetKey(Keyboard::S))
        m_pCamera->m_transform.GoBackward(speed, dt);
    if (GetKey(Keyboard::Q))
        m_pCamera->m_transform.GoLeft(speed, dt);
    if (GetKey(Keyboard::D))
        m_pCamera->m_transform.GoRight(speed, dt);
    //if (GetKey(Keyboard::UP))
    //    m_pCamera->m_transform.GoUp(speed, dt);
    //if (GetKey(Keyboard::DOWN))
    //    m_pCamera->m_transform.GoDown(speed, dt);
    //if (GetKey(Keyboard::LEFT))
    //    m_pCamera->m_transform.GoLeft(speed, dt);
    //if (GetKey(Keyboard::RIGHT))
    //    m_pCamera->m_transform.GoRight(speed, dt);

    //if (GetButton(Mouse::MOUSELEFT))
    //    m_pCamera->m_transform.AddLocalRotation(XMFLOAT3(0.f, 0.f, rollSpeed * dt));
    //if (GetButton(Mouse::MOUSERIGHT))
    //    m_pCamera->m_transform.AddLocalRotation(XMFLOAT3(0.f, 0.f, -rollSpeed * dt));

    if (GetKeyDown(Keyboard::ESCAPE))
		PostQuitMessage(0);
    

    if (GetKeyUp(Keyboard::L)) {
        m_showCursor = !m_showCursor;
        m_cursorLocked = !m_cursorLocked;
    }

	ShowMouseCursor(m_showCursor);

    XMFLOAT2 mouseDelta = LockMouseCursor(m_cursorLocked);

    m_pCamera->m_transform.RotateY(mouseDelta.x * mouseSens * dt);
    m_pCamera->m_transform.Pitch(mouseDelta.y * mouseSens * dt);
    mLastMousePos = mouse;
    m_pCamera->UpdateViewMatrix();
}

void GamePlayApp::CreatePrefabs()
{
    MaterialConstants* constant = new MaterialConstants();

	CreatePrefabRails(constant);
	CreatePrefabStreetLamp(constant);
    CreatePrefabSpotLight(constant);
    CreatePrefabStreetLight(constant);
	CreatePrefabWagon(constant);
    CreatePrefabBullet();
}

void GamePlayApp::CreatePrefabRails(MaterialConstants* material)
{
    material->color = Color::Pink;

    Entity* leftRail = m_em->CreateEntity();
    leftRail->SetName("Rails_LeftRail");
    leftRail->AddComponent<ComponentMeshRenderer>();
    leftRail->GetComponent<ComponentMeshRenderer>()->SetGeometry(GeometryFactory::CreateCylinder(*material));
    leftRail->m_transform.SetOffset(XMFLOAT3(2.f, 0.f, 0.f));
    leftRail->m_transform.SetRotationDegrees(90.f, 0.f, 0.f);
    leftRail->m_transform.SetScale(XMFLOAT3(1.f, 5.f, 1.f));
    leftRail->GetComponent<ComponentMeshRenderer>()->SetOverrideColor(Color::NormalizeColor(Color::Pink));

    Entity* rightRail = m_em->CreateEntity();
    rightRail->SetName("Rails_RightRail");
    rightRail->AddComponent<ComponentMeshRenderer>();
    rightRail->GetComponent<ComponentMeshRenderer>()->SetGeometry(GeometryFactory::CreateCylinder(*material));
    rightRail->m_transform.SetOffset(XMFLOAT3(-2.f, 0.f, 0.f));
    rightRail->m_transform.SetRotationDegrees(90.f, 0.f, 0.f);
    rightRail->m_transform.SetScale(XMFLOAT3(1.f, 5.f, 1.f));
    rightRail->GetComponent<ComponentMeshRenderer>()->SetOverrideColor(Color::NormalizeColor(Color::Pink));

    material->color = Color::LightPink;

    Entity* middlePart = m_em->CreateEntity();
    middlePart->SetName("Rails_MiddlePart");
    middlePart->AddComponent<ComponentMeshRenderer>();
    middlePart->GetComponent<ComponentMeshRenderer>()->SetGeometry(GeometryFactory::CreateCube(*material));
    middlePart->m_transform.SetOffset(XMFLOAT3(0.f, 0.5f, 0.f));
    middlePart->m_transform.SetScale(XMFLOAT3(3.f, 0.25f, 0.5f));
    middlePart->GetComponent<ComponentMeshRenderer>()->SetOverrideColor(Color::NormalizeColor(Color::LightPink));

    Entity* dummyEntity = m_em->CreateEntity();

    Entity* railsGroupEntity = m_em->CreateEntity();
    railsGroupEntity->AddComponent<ComponentBoxCollider>();
    railsGroupEntity->SetName("Rails_Group");
    railsGroupEntity->AddChild(dummyEntity);//ToDo avoid this
    railsGroupEntity->AddChild(leftRail);
    railsGroupEntity->AddChild(rightRail);
    railsGroupEntity->AddChild(middlePart);

    m_em->RegisterPrefab("Rails", std::move(Prefab::FromEntity(*railsGroupEntity)));

    m_em->DestroyEntity(dummyEntity);
    m_em->DestroyEntity(leftRail);
    m_em->DestroyEntity(rightRail);
    m_em->DestroyEntity(middlePart);
    m_em->DestroyEntity(railsGroupEntity);
}

void GamePlayApp::CreatePrefabWagon(MaterialConstants* material)
{
    material->color = Color::Pink;
    Entity* floor = m_em->CreateEntity();
    floor->SetName("Wagon_Floor");
    floor->AddComponent<ComponentMeshRenderer>();
    floor->GetComponent<ComponentMeshRenderer>()->SetGeometry(GeometryFactory::CreateCube(*material));
    floor->m_transform.SetScale(XMFLOAT3(2.f, 0.2f, 2.f));
    floor->m_transform.SetOffset(XMFLOAT3(0.f, 0.f , 0.f));
    floor->GetComponent<ComponentMeshRenderer>()->SetOverrideColor(Color::NormalizeColor(Color::Pink));
    floor->AddComponent<ComponentBoxCollider>();

    material->color = Color::LightPink;

    Entity* backWall = m_em->CreateEntity();
    backWall->SetName("Wagon_BackWall");
    backWall->AddComponent<ComponentMeshRenderer>();
    backWall->GetComponent<ComponentMeshRenderer>()->SetGeometry(GeometryFactory::CreateCube(*material));
    backWall->m_transform.SetScale(XMFLOAT3(2.f, 1.f, 0.1f));
    backWall->m_transform.SetOffset(XMFLOAT3(0.f, 1.f, -2.f));
    backWall->GetComponent<ComponentMeshRenderer>()->SetOverrideColor(Color::NormalizeColor(Color::Pink));
    backWall->AddComponent<ComponentBoxCollider>();

    Entity* frontWall = m_em->CreateEntity();
    frontWall->SetName("Wagon_FrontWall");
    frontWall->AddComponent<ComponentMeshRenderer>();
    frontWall->GetComponent<ComponentMeshRenderer>()->SetGeometry(GeometryFactory::CreateCube(*material));
    frontWall->m_transform.SetScale(XMFLOAT3(2.f, 1.f, 0.1f));
    frontWall->m_transform.SetOffset(XMFLOAT3(0.f, 1.f, 2.f));
    frontWall->GetComponent<ComponentMeshRenderer>()->SetOverrideColor(Color::NormalizeColor(Color::Pink));
    frontWall->AddComponent<ComponentBoxCollider>();

    Entity* leftWall = m_em->CreateEntity();
    leftWall->SetName("Wagon_LeftWall");
    leftWall->AddComponent<ComponentMeshRenderer>();
    leftWall->GetComponent<ComponentMeshRenderer>()->SetGeometry(GeometryFactory::CreateCube(*material));
    leftWall->m_transform.SetScale(XMFLOAT3(0.1f, 1.f, 2.f));
    leftWall->m_transform.SetOffset(XMFLOAT3(-2.f, 1.f, 0.f));
    leftWall->GetComponent<ComponentMeshRenderer>()->SetOverrideColor(Color::NormalizeColor(Color::Pink));
    leftWall->AddComponent<ComponentBoxCollider>();

    Entity* rightWall = m_em->CreateEntity();
    rightWall->SetName("Wagon_RightWall");
    rightWall->AddComponent<ComponentMeshRenderer>();
    rightWall->GetComponent<ComponentMeshRenderer>()->SetGeometry(GeometryFactory::CreateCube(*material));
    rightWall->m_transform.SetScale(XMFLOAT3(0.1f, 1.f, 2.f));
    rightWall->m_transform.SetOffset(XMFLOAT3(2.f, 1.f, 0.f));
    rightWall->GetComponent<ComponentMeshRenderer>()->SetOverrideColor(Color::NormalizeColor(Color::Pink));
    rightWall->AddComponent<ComponentBoxCollider>();

    Entity* dummyEntity = m_em->CreateEntity();

    Entity* wagonGroup = m_em->CreateEntity();
    wagonGroup->AddComponent<ComponentBoxCollider>();
    wagonGroup->SetName("Wagon_Group");

    wagonGroup->AddChild(floor);
    wagonGroup->AddChild(backWall);
    wagonGroup->AddChild(frontWall);
    wagonGroup->AddChild(leftWall);
    wagonGroup->AddChild(rightWall);

    wagonGroup->AddChild(dummyEntity);

    m_em->RegisterPrefab("Wagon", std::move(Prefab::FromEntity(*wagonGroup)));

    m_em->DestroyEntity(floor);
    m_em->DestroyEntity(backWall);
    m_em->DestroyEntity(frontWall);
    m_em->DestroyEntity(leftWall);
    m_em->DestroyEntity(rightWall);

    m_em->DestroyEntity(dummyEntity);

    m_em->DestroyEntity(wagonGroup);
}

void GamePlayApp::CreatePrefabStreetLamp(MaterialConstants* material)
{
    material->color = Color::Black;

    Entity* pillar = m_em->CreateEntity();
    pillar->SetName("StreetLamp_Pillar");
    pillar->AddComponent<ComponentMeshRenderer>();
    pillar->GetComponent<ComponentMeshRenderer>()->SetGeometry(GeometryFactory::CreateCylinder(*material));
    pillar->m_transform.SetOffset(XMFLOAT3(0.f, 0.f, 0.f));
    pillar->m_transform.SetScale(XMFLOAT3(0.5f, 5.f, 0.5f));

    material->color = Color::White;

    Entity* topLight = m_em->CreateEntity();
    topLight->SetName("StreetLamp_TopLight");
    topLight->AddComponent<ComponentMeshRenderer>();
    topLight->GetComponent<ComponentMeshRenderer>()->SetGeometry(GeometryFactory::CreateSphere(*material));
	topLight->AddComponent<ComponentLight>();
    topLight->m_transform.SetOffset(XMFLOAT3(0.f, 2.5f, 0.f));
    topLight->m_transform.SetScale(XMFLOAT3(2.f, 2.f, 2.f));

    auto pointLight = topLight->GetComponent<ComponentLight>();
    pointLight->SetType(LightType::POINT);
    pointLight->SetColor(Color::White);
    pointLight->SetPosition(topLight->m_transform.GetLastWorldPosition().x, topLight->m_transform.GetLastWorldPosition().y, topLight->m_transform.GetLastWorldPosition().z);

    Entity* dummyEntity = m_em->CreateEntity();

	Entity* streetLampGroupEntity = m_em->CreateEntity();
	streetLampGroupEntity->SetName("StreetLamp_Group");
	streetLampGroupEntity->AddChild(dummyEntity);
	streetLampGroupEntity->AddChild(pillar);
	streetLampGroupEntity->AddChild(topLight);

	m_em->RegisterPrefab("StreetLamp", std::move(Prefab::FromEntity(*streetLampGroupEntity)));

    m_em->DestroyEntity(pillar);
    m_em->DestroyEntity(topLight);
}

void GamePlayApp::CreatePrefabStreetLight(MaterialConstants* material)
{
    material->color = Color::Black;

    Entity* Light = m_em->CreateEntity();
    Light->AddComponent<ComponentMeshRenderer>();
    Light->GetComponent<ComponentMeshRenderer>()->SetGeometry(GeometryFactory::CreateSphere(*material));
    Light->AddComponent<ComponentLight>();
    Light->m_transform.SetOffset(XMFLOAT3(0.f, 1.f, -5.f));
    Light->m_transform.SetScale(XMFLOAT3(0.1f, 0.1f, 0.1f));

    auto pointLight = Light->GetComponent<ComponentLight>();
    pointLight->SetType(LightType::POINT);
    pointLight->SetColor(Color::White);
    pointLight->SetIntensity(0.8);
	pointLight->SetRange(50.f);
    //pointLight->SetPosition(Light->m_transform.GetLastWorldPosition().x, Light->m_transform.GetLastWorldPosition().y, Light->m_transform.GetLastWorldPosition().z);

    m_em->RegisterPrefab("StreetLight", std::move(Prefab::FromEntity(*Light)));

    m_em->DestroyEntity(Light);
}

void GamePlayApp::CreatePrefabSpotLight(MaterialConstants* material)
{
    material->color = Color::DarkBlue;

    Entity* spotLight = m_em->CreateEntity();
    spotLight->AddComponent<ComponentMeshRenderer>();
    spotLight->GetComponent<ComponentMeshRenderer>()->SetGeometry(GeometryFactory::CreateSphere(*material));
    spotLight->m_transform.SetOffset(XMFLOAT3(0.f, 1.f, -5.f));
    spotLight->m_transform.SetScale(XMFLOAT3(2.f, 2.f, 2.f));

    auto* lightComp = spotLight->AddComponent<ComponentLight>();
    lightComp->SetType(LightType::SPOT);
    lightComp->SetColor(Color::Pink);
    lightComp->SetIntensity(0.8f);
    lightComp->SetRange(50.f);
    lightComp->SetStrength(2.f);
    lightComp->UseEntityForward(false);
    lightComp->SetDirection(0.f, 0.f, 1.f);
    lightComp->SetAngle(20.f);
    lightComp->SetPenumbra(30.f);

    spotLight->SetActive(false);

    m_em->RegisterPrefab("Projector", std::move(Prefab::FromEntity(*spotLight)));

    m_em->DestroyEntity(spotLight);
}

void GamePlayApp::TextureRegister()
{
	Texture::RegisterTexture("Turtle", "../../res/Texture/turtule.png");
    Texture::RegisterTexture("Chenille", "../../res/Texture/Chenille.png");
    Texture::RegisterTexture("Floor", "../../res/Texture/Floor.png");
}

void GamePlayApp::FontRegister()
{
    FontManager::RegisterFont("Pixel", "../../res/Font/Font_PixeArt.png", 8, 15, 300, 160);
}

void GamePlayApp::CreatePrefabBullet()
{
	Material* material = new Material();

	material->SetColor(Color::Yellow);

	Entity* bullet = m_em->CreateEntity();
	bullet->SetName("Bullet");
	bullet->AddComponent<ComponentMeshRenderer>()->SetGeometry(GeometryFactory::CreateSphere());
    bullet->AddComponent<ComponentBoxCollider>();
	bullet->GetComponent<ComponentMeshRenderer>()->SetMaterial(material);
	bullet->m_transform.SetScale(XMFLOAT3(1.f, 1.f, 1.f));
    bullet->GetComponent<ComponentMeshRenderer>()->SetOverrideColor(Color::NormalizeColor(Color::Yellow));

	m_em->RegisterPrefab("Bullet", std::move(Prefab::FromEntity(*bullet)));

	m_em->DestroyEntity(bullet);
}
