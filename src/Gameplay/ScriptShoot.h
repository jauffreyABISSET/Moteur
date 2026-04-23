#pragma once
#include "Engine/Script.h"
#include <vector>

class ScriptShoot : public Script
{
    struct BulletData
    {
        Entity* entity;
        float timer;
        XMFLOAT3 direction;
    };

    std::vector<BulletData> m_bullets;
    float m_bulletSpeed = 20.f;
    float m_bulletLifetime = 5.f;

public:
    ScriptShoot() = default;
    ~ScriptShoot() override = default;

    void Init() override;
    void Update(float dt) override;
    std::unique_ptr<Script> Clone() const override;

    void Shoot(const XMFLOAT3& dir);
};