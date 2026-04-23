#pragma once
#include "EnemiesConfigs.h"

class Camera;
class StateMachine;

// =======================================================================================================================================================
// ENEMY AI SCRIPT BASE
// =======================================================================================================================================================

class EnemyAIScriptBase : public Script
{
public:
    EnemyAIScriptBase(const EnemyConfig& config);

    virtual void Init() override;
    virtual void Update(float dt) override;
    std::unique_ptr<Script> Clone() const override;

protected:
    virtual void SetupStateMachine();
    float DistanceToPlayer() const;

    virtual void OnAttackEnter(Entity*) {}
    virtual void OnAttackUpdate(Entity*, float) {}
    virtual void OnAttackExit(Entity*) {}

protected:
    EnemyConfig m_config;
    StateMachine* m_stateMachine = nullptr;
    Camera* m_playerCamera = nullptr;
    float m_currentAttackCooldown = 0.f;
};

// =======================================================================================================================================================
// GHOST ENEMY SCRIPT
// =======================================================================================================================================================

class GhostEnemyScript : public EnemyAIScriptBase
{
public:
    GhostEnemyScript() : EnemyAIScriptBase(GhostEnemyConfig()) {};

private:
    void OnAttackEnter(Entity* e) override;
    void OnAttackExit(Entity* e) override;
};

// =======================================================================================================================================================
// SHOOTER ENEMY SCRIPT
// =======================================================================================================================================================

class ShooterEnemyScript : public EnemyAIScriptBase
{
public:
    ShooterEnemyScript() : EnemyAIScriptBase(ShooterEnemyConfig()) {}

private:
    void OnAttackEnter(Entity* e) override;
    void OnAttackExit(Entity* e) override;
};

// =======================================================================================================================================================
// RAIL BASE ENEMY SCRIPT
// =======================================================================================================================================================

class RailEnemyBaseScript : public EnemyAIScriptBase
{
public:
    RailEnemyBaseScript();

    void Init() override;
    void Update(float dt) override;

protected:
    std::vector<Entity*> m_rails;
    float m_railIndexFloat;

    virtual float RailDirection() const = 0;

    void MoveOnRail(float dt);

    void OnAttackEnter(Entity* e) override;
    void OnAttackExit(Entity* e) override;
};

// =======================================================================================================================================================
// RAIL FORWARD ENEMY SCRIPT
// =======================================================================================================================================================

class RailEnemyForwardScript : public RailEnemyBaseScript
{
public:
    float RailDirection() const override;
};

// =======================================================================================================================================================
// RAIL BACKWARD ENEMY SCRIPT
// =======================================================================================================================================================

class RailEnemyBackwardScript : public RailEnemyBaseScript
{
public:
    float RailDirection() const override;
};