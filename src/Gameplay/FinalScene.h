#pragma once
#include "../Engine/AppManager.h"

class FinalScene : public Scene
{
private:

    XMFLOAT2 mLastMousePos = { 0.f, 0.f };

    GameManager* gm = nullptr;
    EntityManager* em = nullptr;
    SceneManager* sm = nullptr;
    LightManager* lm = nullptr;
    Camera* m_pCamera = nullptr;

    std::vector<Entity*> m_loadedRails;
    std::vector<Entity*> m_instance1;

	Entity* m_player;

    float m_spawnRailTimer = 0.f;
    float m_spawnRailInterval = 0.5f;

    Entity* m_wagon = nullptr;
    float m_cameraHeight = 2.0f;
    Entity* m_cameraLightEntity = nullptr;
    Entity* m_followingWagon = nullptr;
    // Smooth follow
    float m_railIndexFloat = 0.f;
    bool  m_followRails = false;

    // Last rail saved
    XMFLOAT3 m_lastRailPos = XMFLOAT3();
    XMFLOAT4 m_lastRailQuat = XMFLOAT4();
    bool m_hasLastRail = false;
    float m_railSpeed = 2.0f;
    float m_railOffsetY = 3.0f;
    bool m_cameraLocked = false;
    int entityCount = 0;

    Entity* m_HPTextEntity = nullptr;

    int m_playerScore = -500; 
    Entity* m_score = nullptr;
    int m_playerHP = 0;

    DirectX::XMFLOAT3 m_cameraLocalOffsetPos{ 0.f, 0.f, 0.f };
    DirectX::XMFLOAT4 m_cameraLocalOffsetQuat{ 0.f, 0.f, 0.f, 1.f };
    bool m_cameraOffsetStored = false;

    float m_cameraHeightOffset = 2.0f;     // Hauteur au-dessus du rail
    float m_cameraForwardOffset = 0.0f;    // Décalage avant/arrière
    float m_cameraSideOffset = 0.0f;


    // map des quaternions locaux pour les enfants du wagon
    std::unordered_map<Entity*, DirectX::XMFLOAT4> m_wagonLocalRotations;
public:
    FinalScene() = default;
    virtual ~FinalScene();

    void Init() override;
    void Update(const GameTimer& gt) override;
    void UpdateRailDistance();
    void UpdateEnemyDistance();
    void HandleRailFollow(float dt);
    void CreateRails(float dt);
    void Draw(const GameTimer& gt) override;
    std::unordered_map<Entity*, DirectX::XMFLOAT4> m_railBaseColors;
    void CreateEnemies();
    void CreateUI();

    int GetPlayerScore();
    void SetPlayerScore(int value);
    void AddToPlayerScore(int value);

    int GetPlayerHP();
    void SetPlayerHP(int value);
    void AddToPlayerHP(int value);

    void IsPlayerAlive();
    void EndTheGame();
};