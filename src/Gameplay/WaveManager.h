#pragma once

class WaveManagerScript: public Script
{
public:
    void Init() override;
    void Update(float dt) override;

    void SetLoadedRails(std::vector<Entity*> loadedRails);

    std::unique_ptr<Script> Clone() const override;

private:
    void SpawnWave();
    void SpawnOneEnemy();

    XMFLOAT3 RandomSpawnAroundPlayer();

private:
    int m_currentWave = 0;
    int m_lastWaveEnemyCount = 0;

    float m_waveTimer = 0.f;
    float m_waveDuration = 50.f;

    std::vector<Entity*> m_aliveEnemies;

    std::vector<Entity*> m_loadedRails;
};

int GetRandomNumber(int min, int max);