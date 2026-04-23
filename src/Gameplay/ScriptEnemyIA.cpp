#include "pch.h"
#include "ScriptEnemyIA.h"
#include "ScriptShoot.h"
#include "FinalScene.h"

// =======================================================================================================================================================
// ENEMY AI SCRIPT BASE
// =======================================================================================================================================================

EnemyAIScriptBase::EnemyAIScriptBase(const EnemyConfig& config) : m_config(config)
{
    m_stateMachine = nullptr;
    m_playerCamera = nullptr;
    m_currentAttackCooldown = 0.f;
}

void EnemyAIScriptBase::Init()
{
    m_playerCamera = RenderSystem::Get().GetActiveCamera();
    m_stateMachine = GameManager::GetInstance()->CreateStateMachine(m_pEntity);

    m_currentAttackCooldown = m_config.attackCooldown;

    SetupStateMachine();
}


void EnemyAIScriptBase::Update(float dt)
{
	m_pEntity->m_transform.LookAt(m_playerCamera->m_transform.GetWorldPosition());
}

std::unique_ptr<Script> EnemyAIScriptBase::Clone() const
{
    return std::make_unique<EnemyAIScriptBase>(*this);
}

float EnemyAIScriptBase::DistanceToPlayer() const
{
    if (!m_playerCamera)
        return FLT_MAX;

    XMFLOAT3 enemyPos = m_pEntity->m_transform.GetWorldPosition();
    XMFLOAT3 playerPos = m_playerCamera->m_transform.GetWorldPosition();

    float dx = enemyPos.x - playerPos.x;
    float dy = enemyPos.y - playerPos.y;
    float dz = enemyPos.z - playerPos.z;

    return sqrtf(dx * dx + dy * dy + dz * dz);
}

void EnemyAIScriptBase::SetupStateMachine()
{
    // =========================
    // IDLE
    // =========================
    m_stateMachine->AddState(
        "Idle",
        nullptr,
        [this](Entity*, float dt)
        {
            m_currentAttackCooldown += dt;
        },
        nullptr
    );

    // =========================
    // CHASE
    // =========================
    m_stateMachine->AddState(
        "Chase",
        nullptr,
        [this](Entity* e, float dt)
        {
            XMFLOAT3 enemyPos = e->m_transform.GetWorldPosition();
            XMFLOAT3 playerPos = m_playerCamera->m_transform.GetWorldPosition();

            XMVECTOR ePos = XMLoadFloat3(&enemyPos);
            XMVECTOR pPos = XMLoadFloat3(&playerPos);

            XMVECTOR dir = XMVector3Normalize(pPos - ePos);
            XMVECTOR newPos = ePos + dir * m_config.speed * dt;

            XMFLOAT3 out;
            XMStoreFloat3(&out, newPos);

            e->m_transform.SetPosition(out);
        },
        nullptr
    );

    // =========================
    // ATTACK
    // =========================
    m_stateMachine->AddState(
        "Attack",
        [this](Entity* e, float)
        {
            OnAttackEnter(e);
        },
        [this](Entity* e, float dt)
        {
            m_currentAttackCooldown += dt;
            OnAttackUpdate(e, dt);
        },
        [this](Entity* e, float)
        {
            m_currentAttackCooldown = 0.f;
            OnAttackExit(e);
        }
    );

    // =========================
    // TRANSITIONS
    // =========================

    // Idle -> Chase
    m_stateMachine->AddTransition(
        "Idle",
        "Chase",
        {
            [this](Entity*, float)
            {
                return DistanceToPlayer() <= m_config.followDistance || m_config.followDistance < 0.f;
            }
        }
    );

    // Chase -> Attack
    m_stateMachine->AddTransition(
        "Chase",
        "Attack",
        {
            [this](Entity*, float)
            {
                return DistanceToPlayer() <= m_config.attackDistance;
            }
        }
    );

    // Attack -> Idle
    m_stateMachine->AddTransition(
        "Attack",
        "Idle",
        {
            [this](Entity*, float)
            {
                return m_currentAttackCooldown >= m_config.attackCooldown;
            }
        }
    );

    // Chase -> Idle
    m_stateMachine->AddTransition(
        "Chase",
        "Idle",
        {
            [this](Entity*, float)
            {
                return DistanceToPlayer() > m_config.followDistance && m_config.followDistance >= 0.f;
            }
        }
    );

    m_stateMachine->SetInitialState("Idle");
}

// =======================================================================================================================================================
// GHOST ENEMY SCRIPT
// =======================================================================================================================================================

void GhostEnemyScript::OnAttackEnter(Entity* e)
{
    // HIT THE PLAYER
    GameManager::GetInstance()->GetSceneManager().GetScene<FinalScene>()->AddToPlayerHP(-m_config.damage);
}

void GhostEnemyScript::OnAttackExit(Entity* e)
{
    e->GetComponent<ComponentMeshRenderer>()->GetMaterial()->SetColor(m_config.baseColor);
}

// =======================================================================================================================================================
// SHOOTER ENEMY SCRIPT
// =======================================================================================================================================================

void ShooterEnemyScript::OnAttackEnter(Entity* e)
{
    e->GetComponent<ComponentMeshRenderer>()->GetMaterial()->SetColor(Color::DarkBlue);

    auto shootScript = e->GetScript<ScriptShoot>();
    if (shootScript && m_playerCamera)
    {
        // Direction vers le joueur
        XMFLOAT3 enemyPos = e->m_transform.GetWorldPosition();
        XMFLOAT3 playerPos = m_playerCamera->m_transform.GetWorldPosition();

        XMFLOAT3 dir = {
            playerPos.x - enemyPos.x,
            playerPos.y - enemyPos.y,
            playerPos.z - enemyPos.z
        };

        // Normaliser
        XMVECTOR dirV = XMLoadFloat3(&dir);
        dirV = XMVector3Normalize(dirV);
        XMStoreFloat3(&dir, dirV);

        shootScript->Shoot(dir);
    }
}

void ShooterEnemyScript::OnAttackExit(Entity* e)
{
    e->GetComponent<ComponentMeshRenderer>()->GetMaterial()->SetColor(m_config.baseColor);
}

// =======================================================================================================================================================
// RAIL BASE ENEMY SCRIPT
// =======================================================================================================================================================

void RailEnemyBaseScript::Init()
{
    EnemyAIScriptBase::Init();

    Scene* scene = GameManager::GetInstance()->GetSceneManager().GetCurrentScene();
    if (!scene)
        return;

    for (Entity* e : scene->GetEntitiesOfThisScene())
    {
        if (e && e->GetName() == "Rails" && !e->HasParent())
            m_rails.push_back(e);
    }

    m_railIndexFloat = 0.f;
}

void RailEnemyBaseScript::Update(float dt)
{
    if (m_rails.empty())
        return;

    EnemyAIScriptBase::Update(dt);

    MoveOnRail(dt);
}

void RailEnemyBaseScript::MoveOnRail(float dt)
{
    m_railIndexFloat += RailDirection() * m_config.speed * dt;

    int railCount = static_cast<int>(m_rails.size());

    if (m_railIndexFloat < 0.f)
        m_railIndexFloat += railCount;
    else if (m_railIndexFloat >= railCount)
        m_railIndexFloat -= railCount;

    int index0 = static_cast<int>(floor(m_railIndexFloat));
    int index1 = (index0 + (RailDirection() > 0 ? 1 : -1) + railCount) % railCount;

    float t = m_railIndexFloat - index0;
    if (t < 0.f) t += 1.f;

    // === POSITIONS ===
    float railHeightOffset = 2.5f;

    XMFLOAT3 pos0 = m_rails[index0]->m_transform.GetWorldPosition();
    XMFLOAT3 pos1 = m_rails[index1]->m_transform.GetWorldPosition();

    XMFLOAT3 interpPos = XMFLOAT3();
    interpPos.x = pos0.x + (pos1.x - pos0.x) * t;
    interpPos.y = pos0.y + (pos1.y - pos0.y) * t;
    interpPos.z = pos0.z + (pos1.z - pos0.z) * t;

    // === ROTATION ===
    XMFLOAT4 q0 = m_rails[index0]->m_transform.GetQuat();
    XMFLOAT4 q1 = m_rails[index1]->m_transform.GetQuat();

    XMVECTOR quat0 = XMLoadFloat4(&q0);
    XMVECTOR quat1 = XMLoadFloat4(&q1);
    XMVECTOR interpQuat = XMQuaternionSlerp(quat0, quat1, t);
    XMVECTOR up = XMVector3Rotate(XMVectorSet(0.f, 1.f, 0.f, 0.f), interpQuat);

    // === APPLY OFFSET ===
    XMVECTOR basePos = XMLoadFloat3(&interpPos);
    XMVECTOR finalPos = basePos + up * railHeightOffset;

    XMFLOAT3 outPos;
    XMStoreFloat3(&outPos, finalPos);

    // === APPLY TRANSFORM ===
    m_pEntity->m_transform.SetPosition(outPos);

    XMFLOAT4 outQuat;
    XMStoreFloat4(&outQuat, interpQuat);
    m_pEntity->m_transform.SetQuat(outQuat);
}

void RailEnemyBaseScript::OnAttackEnter(Entity* e)
{
    GameManager::GetInstance()->GetSceneManager().GetScene<FinalScene>()->AddToPlayerHP(-m_config.damage);
}

void RailEnemyBaseScript::OnAttackExit(Entity* e)
{
    e->GetComponent<ComponentMeshRenderer>()->GetMaterial()->SetColor(m_config.baseColor);
}

RailEnemyBaseScript::RailEnemyBaseScript() : EnemyAIScriptBase(RailEnemyConfig())
{
    m_railIndexFloat = 0.f;
}

// =======================================================================================================================================================
// RAIL FORWARD ENEMY SCRIPT
// =======================================================================================================================================================

float RailEnemyForwardScript::RailDirection() const
{
    return 1.0f;
}

// =======================================================================================================================================================
// RAIL BACKWARD ENEMY SCRIPT
// =======================================================================================================================================================

float RailEnemyBackwardScript::RailDirection() const
{
    return -1.0f;
}