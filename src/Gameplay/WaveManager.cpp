#include "pch.h"
#include "WaveManager.h"
#include "EnemyFactory.h"
#include "FinalScene.h"
#include <random>

void WaveManagerScript::Init()
{
}

void WaveManagerScript::Update(float dt)
{
    m_waveTimer -= dt;

    m_aliveEnemies.erase(
        std::remove_if(
            m_aliveEnemies.begin(),
            m_aliveEnemies.end(),
            [](Entity* e)
            {
                return !e || !e->IsActive();
            }),
        m_aliveEnemies.end()
    );

    if (GetKeyUp(Keyboard::G))
    {
        for (Entity* entity : m_aliveEnemies)
        {
            entity->SetActive(false);
        }
    }

    if (m_aliveEnemies.empty() || m_waveTimer <= 0.f)
    {
        SpawnWave();

        GameManager::GetInstance()->GetSceneManager().GetScene<FinalScene>()->AddToPlayerScore(500);
    }
}

void WaveManagerScript::SetLoadedRails(std::vector<Entity*> loadedRails)
{
    m_loadedRails = loadedRails;
}

int GetRandomNumber(int min, int max)
{
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

void WaveManagerScript::SpawnOneEnemy()
{
    Scene* scene = GameManager::GetInstance()->GetSceneManager().GetCurrentScene();

    if (!scene)
        return;

    int enemyTypeRoll = GetRandomNumber(0, 100);
    Entity* enemy = nullptr;

    XMFLOAT3 spawnPos = RandomSpawnAroundPlayer();

    if (m_currentWave < 3)
    {
        enemy = EnemyFactory::SpawnGhost(scene, spawnPos);
    }
    else if (m_currentWave < 5)
    {
        if (enemyTypeRoll < 70)
            enemy = EnemyFactory::SpawnGhost(scene, spawnPos);
        else
            enemy = EnemyFactory::SpawnEnemyShooter(scene, spawnPos);
    }
    else
    {
        // === GHOST ===
        if (enemyTypeRoll < 50)
        {
            enemy = EnemyFactory::SpawnGhost(scene, spawnPos);
        }
        // === SHOOTER ===
        else if (enemyTypeRoll < 80)
        {
            enemy = EnemyFactory::SpawnEnemyShooter(scene, spawnPos);
        }
        // === RAIL ENEMY ===
        else
        {
            bool forward = GetRandomNumber(0, 1);

            XMFLOAT3 pos = m_loadedRails[GetRandomNumber(0, m_loadedRails.size() - 1)]->m_transform.GetWorldPosition();

            enemy = forward
                ? EnemyFactory::SpawnEnemyRailForward(scene, pos)
                : EnemyFactory::SpawnEnemyRailBackward(scene, pos);
        }
    }

    if (enemy)
        m_aliveEnemies.push_back(enemy);
}

XMFLOAT3 WaveManagerScript::RandomSpawnAroundPlayer()
{
    XMFLOAT3 pos = XMFLOAT3(0.f, 0.f, 0.f);

    pos.x = GetRandomNumber(0, 200);
    pos.y = GetRandomNumber(-50, 50);
    pos.z = GetRandomNumber(0, 200);

    return pos;
}

std::unique_ptr<Script> WaveManagerScript::Clone() const
{
    return std::unique_ptr<WaveManagerScript>();
}

void WaveManagerScript::SpawnWave()
{
    m_currentWave++;
    m_waveTimer = m_waveDuration;

    int minEnemies = m_lastWaveEnemyCount;
    int maxEnemies = m_lastWaveEnemyCount + 3;

    if (m_currentWave == 1)
        minEnemies = maxEnemies = 2;

    int enemyCount = GetRandomNumber(minEnemies, maxEnemies);
    m_lastWaveEnemyCount = enemyCount;

    for (int i = 0; i < enemyCount; ++i)
        SpawnOneEnemy();
}