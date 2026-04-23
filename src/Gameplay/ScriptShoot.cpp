#include "pch.h"
#include "ScriptShoot.h"
#include "FinalScene.h"
#include "EnemiesConfigs.h"

void ScriptShoot::Init()
{
    // Rien de spécial à initialiser
}

void ScriptShoot::Update(float dt)
{
    if (!m_pEntity) return;

    if (m_pEntity->GetName() == "Player" && GetButtonDown(Mouse::MOUSELEFT))
    {
        XMVECTOR forward = RenderSystem::Get().GetActiveCamera()->m_transform.GetLook();
        XMFLOAT3 dir;
        XMStoreFloat3(&dir, forward);
        Shoot(dir);
    }

    GameManager* gm = GameManager::GetInstance();
    if (!gm) return;
    Scene* current = gm->GetSceneManager().GetCurrentScene();
    if (!current) return;

    for (auto& b : m_bullets)
    {
        if (!b.entity) continue;

        XMFLOAT3 currentPos = b.entity->m_transform.GetWorldPosition();

        XMVECTOR pos = XMLoadFloat3(&currentPos);
        XMVECTOR move = XMLoadFloat3(&b.direction) * m_bulletSpeed * dt;
        pos += move;
        XMFLOAT3 newPos;
        XMStoreFloat3(&newPos, pos);
        b.entity->m_transform.SetPosition(newPos);

        // Vérification de collision simple bullet -> ennemis
        for (Entity* e : current->GetEntitiesOfThisScene())
        {
            if (!e) continue;
            const std::string& name = e->GetName();
            if (name.find("Enemy") == std::string::npos && name.find("Ghost") == std::string::npos && name.find("Shooter") == std::string::npos && name.find("RailEnemy") == std::string::npos)
                continue;

            if (!e->IsActive() || e->IsDestroyed()) continue;

            XMFLOAT3 entPos = e->m_transform.GetWorldPosition();
            float dx = entPos.x - newPos.x;
            float dy = entPos.y - newPos.y;
            float dz = entPos.z - newPos.z;
            float distSq = dx*dx + dy*dy + dz*dz;

            // seuil de collision approximatif (ajuste si besoin)
            const float collisionRadius = 4.0f; // valeur générale ; réduire/augmenter selon taille ennemis
            if (distSq <= collisionRadius * collisionRadius)
            {
                // Détruire la balle
                b.entity->SetActive(false);
                b.entity->SetDestroyed(true);
                b.entity->Destroy();
                b.entity = nullptr;

                // Détruire l'ennemi et ajouter les points correspondants
                int points = 0;
                if (name.find("Ghost") != std::string::npos) points = GhostEnemyConfig().pointsGiven;
                else if (name.find("Shooter") != std::string::npos) points = ShooterEnemyConfig().pointsGiven;
                else if (name.find("RailEnemy") != std::string::npos) points = RailEnemyConfig().pointsGiven;
                else if (name.find("Enemy") != std::string::npos) points = 100;

                // Ajouter les points et détruire l'ennemi
                auto finalScene = gm->GetSceneManager().GetScene<FinalScene>();
                if (finalScene) finalScene->AddToPlayerScore(points);

                e->SetActive(false);
                e->SetDestroyed(true);
                e->Destroy();

                // sortir de la boucle d'ennemis pour cette balle (elle est détruite)
                break;
            }
        }

        // Timer
        if (b.entity) // si pas déjà détruite par collision
        {
            b.timer += dt;
            if (b.timer >= m_bulletLifetime)
            {
                b.entity->SetActive(false);
                b.entity->SetDestroyed(true);
                b.entity->Destroy();
                b.entity = nullptr;
            }
        }
    }

    // Nettoyer les balles détruites
    m_bullets.erase(
        std::remove_if(m_bullets.begin(), m_bullets.end(), [](const BulletData& b) { return b.entity == nullptr; }),
        m_bullets.end()
    );
}

void ScriptShoot::Shoot(const XMFLOAT3& dir)
{
    if (!m_pEntity) return;

    GameManager* gm = GameManager::GetInstance();
    if (!gm) return;

    Scene* current = gm->GetSceneManager().GetCurrentScene();
    if (!current) return;

    // Offset pour spawner la balle un peu devant l'entité (player)
    const float spawnOffset = 2.5f;

    // Calculer position de spawn basée sur la direction fournie
    XMFLOAT3 spawnPos = m_pEntity->m_transform.GetWorldPosition();
    XMVECTOR dirV = XMLoadFloat3(&dir);
    // protéger contre vecteur nul
    if (XMVector3Equal(dirV, XMVectorZero()) == FALSE)
    {
        dirV = XMVector3Normalize(dirV);
        XMVECTOR spawnV = XMLoadFloat3(&spawnPos) + dirV * spawnOffset;
        XMStoreFloat3(&spawnPos, spawnV);
    }

    auto created = current->CreatePrefabs("Bullet", spawnPos, m_pEntity->m_transform.GetQuat());
    Entity* bulletEntity = nullptr;

    if (!created.empty())
    {
        bulletEntity = created[0];
    }
    else
    {
        bulletEntity = current->CreateEntity();
        if (bulletEntity)
        {
            bulletEntity->SetName("Bullet_Fallback");
            bulletEntity->m_transform = m_pEntity->m_transform;
            bulletEntity->m_transform.SetPosition(spawnPos);
            bulletEntity->SetActive(true);
        }
    }

    if (bulletEntity)
    {
        m_bullets.push_back({ bulletEntity, 0.f, dir });
    }
}

std::unique_ptr<Script> ScriptShoot::Clone() const
{
    return std::make_unique<ScriptShoot>(*this);
}