#pragma once
#include "Engine/Scene.h"
#include "Render/Camera.h"

class Prefab;

class GamePlayScene : public Scene
{
    XMFLOAT2 mLastMousePos = { 0.f, 0.f };
	GameManager* gm = nullptr;
    EntityManager* em = nullptr;
	SceneManager* sm = nullptr;
	LightManager* lm = nullptr;

	Prefab* m_prefab;
	std::vector<Entity*> m_instance1;

	bool test = true;
    bool switchOrbit = true;

    void RandomTestEntity();

public:
    GamePlayScene() = default;
    virtual ~GamePlayScene();

    void Init() override;
    void Update(const GameTimer& gt) override;
    void Draw(const GameTimer& gt) override;
};

class ScriptTest : public Script
{
 public:
    void Init() override;
    void Update(float dt) override;
    std::unique_ptr<Script> Clone() const override;
};