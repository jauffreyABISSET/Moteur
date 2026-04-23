#include "pch.h"
#include "EnemyFactory.h"
#include "ScriptEnemyIA.h"
#include "ScriptShoot.h"

Entity* EnemyFactory::SpawnGhost(Scene* scene, const XMFLOAT3& pos)
{
    Entity* enemy = scene->CreateEntity();

    enemy->SetName("GhostEnemy");

    enemy->m_transform.SetPosition(pos);
    enemy->m_transform.SetScale(7.f, 7.f, 7.f);

    auto mesh = enemy->AddComponent<ComponentMeshRenderer>();
    mesh->SetGeometry(GeometryFactory::CreateSphere());
    mesh->SetOverrideColor(GhostEnemyConfig().baseColor);

    auto script = enemy->AddScript<GhostEnemyScript>();

    return enemy;
}

Entity* EnemyFactory::SpawnEnemyShooter(Scene* scene, const XMFLOAT3& pos)
{
    Entity* enemy = scene->CreateEntity();

    enemy->SetName("ShooterEnemy");

    enemy->m_transform.SetPosition(pos);
    enemy->m_transform.SetScale(4.f, 4.f, 4.f);
    enemy->AddScript<ScriptShoot>();

    auto mesh = enemy->AddComponent<ComponentMeshRenderer>();
    mesh->SetGeometry(GeometryFactory::CreateCube());
    mesh->SetOverrideColor(ShooterEnemyConfig().baseColor);

    enemy->AddScript<ShooterEnemyScript>();

    return enemy;
}

Entity* EnemyFactory::SpawnEnemyRailForward(Scene* scene, const XMFLOAT3& pos)
{
    Entity* enemy = scene->CreateEntity();

    enemy->SetName("RailEnemyForward");

    enemy->m_transform.SetPosition(pos);
    enemy->m_transform.SetScale(4.f, 4.f, 4.f);

    auto mesh = enemy->AddComponent<ComponentMeshRenderer>();
    mesh->SetGeometry(GeometryFactory::CreateCylinder());
    mesh->SetOverrideColor(RailEnemyConfig().baseColor);

    enemy->AddScript<RailEnemyForwardScript>();

    return enemy;
}

Entity* EnemyFactory::SpawnEnemyRailBackward(Scene* scene, const XMFLOAT3& pos)
{
    Entity* enemy = scene->CreateEntity();

    enemy->SetName("RailEnemyBackward");

    enemy->m_transform.SetPosition(pos);
    enemy->m_transform.SetScale(4.f, 4.f, 4.f);

    auto mesh = enemy->AddComponent<ComponentMeshRenderer>();
    mesh->SetGeometry(GeometryFactory::CreateCylinder());
    mesh->SetOverrideColor(RailEnemyConfig().baseColor);

    enemy->AddScript<RailEnemyBackwardScript>();

    return enemy;
}