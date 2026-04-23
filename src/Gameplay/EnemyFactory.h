#pragma once

class Scene;
class Entity;

class EnemyFactory
{
public:

    static Entity* SpawnGhost(Scene* scene, const XMFLOAT3& pos);

    static Entity* SpawnEnemyShooter(Scene* scene, const XMFLOAT3& pos);

    static Entity* SpawnEnemyRailForward(Scene* scene, const XMFLOAT3& pos);

    static Entity* SpawnEnemyRailBackward(Scene* scene, const XMFLOAT3& pos);
};