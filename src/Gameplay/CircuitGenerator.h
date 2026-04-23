#pragma once
#include "Engine/Scene.h"

class CircuitGenerator : public Scene
{
	std::list<XMFLOAT3> m_circuitPoints;
	XMFLOAT3 m_lastRailPos;
	XMFLOAT4 m_lastRailQuat;
	bool m_hasLastRail = false;
	bool m_test = true;
	Camera* m_pCamera = nullptr;

	std::vector<Entity*> m_instance1;

	bool test = true;
	bool switchOrbit = true;

	void RandomTestEntity();

	float m_spawnRailTimer = 0.f;
	float m_spawnRailInterval = 0.5f;

public:
	CircuitGenerator() = default;

    void Init() override;
    void Update(const GameTimer& gt) override;
    void Draw(const GameTimer& gt) override;
	void InputScene(float dt);
};

