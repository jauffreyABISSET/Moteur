#include "pch.h"
#include "Prefab.h"
#include <DirectXMath.h>

using namespace DirectX;

Prefab Prefab::FromEntity(const Entity& e)
{
    Prefab prefab;
    prefab.m_transform = e.m_transform;

    if (e.HasComponent()) 
    {
        for (const auto& c : e.GetComponents())
        {
            if (c) prefab.m_components.push_back(c->Clone());
        }
    }

    if (e.HasScript()) 
    {
        for (const auto& s : e.GetScripts())
        {
            if (s) prefab.m_scripts.push_back(s->Clone());
        }
    }

    if (e.HasChild())
    {
        for (auto& child : e.GetChildren())
        {
            prefab.m_children.push_back(Prefab::FromEntity(*child));
        }
    }

    if (!e.GetName().empty())
        prefab.m_name = e.GetName();

    return prefab;
}

void Prefab::ApplyTo(Entity& target, bool asRoot, const XMFLOAT3* parentWorldPos, const XMFLOAT4* parentWorldQuat) const
{
    EntityManager& em = GameManager::GetInstance()->GetEntityManager();

    // --- Appliquer le transform du prefab ---
    target.m_transform.SetScale(m_transform.GetScale());
    target.m_transform.SetOffset(m_transform.GetOffset());

    XMFLOAT4 localQuat = m_transform.GetLocalRotationQuaternion();
    XMVECTOR localQV = XMLoadFloat4(&localQuat);
    localQV = XMQuaternionNormalize(localQV);

    if (asRoot)
    {
        if (parentWorldPos)
            target.m_transform.SetPosition(*parentWorldPos);
        else
            target.m_transform.SetPosition(m_transform.GetWorldPosition());

        if (parentWorldQuat)
        {
            XMVECTOR pwq = XMLoadFloat4(parentWorldQuat);
            XMVECTOR worldQv = XMQuaternionNormalize(XMQuaternionMultiply(pwq, localQV));
            XMFLOAT4 worldQf; XMStoreFloat4(&worldQf, worldQv);
            target.m_transform.SetQuat(worldQf);
        }
        else
        {
            target.m_transform.SetQuat(m_transform.GetLocalRotationQuaternion());
        }
    }

    if (!m_name.empty())
        target.SetName(m_name);

    // --- Ajouter les composants ---
    for (const auto& comp : m_components)
    {
        if (!comp) continue;

        auto clonedComp = comp->Clone();

        // Si c'est un MeshRenderer, on partage le matériau mais chaque instance aura son overrideColor
        if (auto* mesh = dynamic_cast<ComponentMeshRenderer*>(clonedComp.get()))
        {
            if (mesh->GetMaterial())
            {
                // Ne clone pas le matériau, partage le même
                mesh->SetMaterial(mesh->GetMaterial());
                // Couleur override à définir plus tard avec SetOverrideColor()
            }
        }

        target.AddComponent(std::move(clonedComp));
    }

    // --- Ajouter les scripts ---
    for (const auto& script : m_scripts)
    {
        if (script)
            target.AddScript(script->Clone());
    }

    // --- Appliquer récursivement aux enfants ---
    XMFLOAT3 parentPos = target.m_transform.GetWorldPosition();
    XMFLOAT4 parentQuat = target.m_transform.GetQuat();
    XMVECTOR parentQuatV = XMLoadFloat4(&parentQuat);

    for (const Prefab& childPrefab : m_children)
    {
        // Utiliser l'offset local du prefab (c'est celui utilisé lors de la création via SetOffset).
        // Si l'offset est nul, retomber sur GetLocalPosition() (cas où SetPosition a été utilisé).
        XMFLOAT3 localOffset = childPrefab.m_transform.GetOffset();
        const float eps = 1e-6f;
        if (fabs(localOffset.x) < eps && fabs(localOffset.y) < eps && fabs(localOffset.z) < eps)
        {
            localOffset = childPrefab.m_transform.GetLocalPosition();
        }

        XMVECTOR localV = XMLoadFloat3(&localOffset);
        XMVECTOR rotated = XMVector3Rotate(localV, parentQuatV);
        XMFLOAT3 rotatedF; XMStoreFloat3(&rotatedF, rotated);

        XMFLOAT3 childWorldPos{
            parentPos.x + rotatedF.x,
            parentPos.y + rotatedF.y,
            parentPos.z + rotatedF.z
        };

        XMFLOAT4 childLocalQuat = childPrefab.m_transform.GetLocalRotationQuaternion();
        XMVECTOR childLocalQV = XMLoadFloat4(&childLocalQuat);
        XMVECTOR childWorldQV = XMQuaternionNormalize(XMQuaternionMultiply(parentQuatV, childLocalQV));
        XMFLOAT4 childWorldQuatF; XMStoreFloat4(&childWorldQuatF, childWorldQV);

        // Créer l'entité enfant.
        // IMPORTANT : ne PAS positionner l'enfant directement à childWorldPos puis appliquer l'offset ensuite,
        // cela double-applique l'offset et provoque des décalages (roues "tombant" sous le wagon).
        // On positionne l'enfant à la position du parent, et on affecte l'offset local.
        Entity* child = em.CreateEntity();
        child->m_transform.SetPosition(parentPos);            // position de base = parent (offset fera le reste)
        child->m_transform.SetOffset(localOffset);           // offset local = position relative attendue
        child->m_transform.SetQuat(childWorldQuatF);         // orientation correcte

        // On passe childWorldPos / childWorldQuatF pour que les descendants conservent le monde comme parent
        childPrefab.ApplyTo(*child, false, &childWorldPos, &childWorldQuatF);

        child->SetParent(&target);
        target.AddChild(child);
    }
}

void Prefab::ApplyComponents(Entity& target) const
{
    for (const auto& comp : m_components)
    {
        if (comp) target.AddComponent(comp->Clone());
    }
}

const Transform& Prefab::GetTransform() const
{
    return m_transform;
}

const std::string& Prefab::GetName() const
{
    return m_name;
}

const std::vector<Prefab>& Prefab::GetChildren() const
{
    return m_children;
}

void Prefab::AddChild(Prefab child)
{
	m_children.push_back(std::move(child));
}

void Prefab::AddComponent(std::unique_ptr<Component> comp)
{
    if (comp)
        m_components.push_back(std::move(comp));
}

void Prefab::AddScript(std::unique_ptr<Script> script)
{
    if (script)
        m_scripts.push_back(std::move(script));
}

Entity* Prefab::GetParent() const
{
    return m_parent;
}

bool Prefab::HasParent() const
{
    return m_parent != nullptr;
}

void Prefab::SetParent(Entity* parent)
{
    m_parent = parent;
}

void Prefab::Serialize(nlohmann::json& j) const
{
    j["name"] = m_name;

    j["position"] = {
        m_transform.GetWorldPosition().x,
        m_transform.GetWorldPosition().y,
        m_transform.GetWorldPosition().z
    };

    XMFLOAT4 q = m_transform.GetLocalRotationQuaternion();
    j["rotation"] = { q.x, q.y, q.z, q.w };

    j["scale"] = {
        m_transform.GetScale().x,
        m_transform.GetScale().y,
        m_transform.GetScale().z
    };

    j["components"] = nlohmann::json::array();
    for (const auto& comp : m_components)
    {
        if (!comp) continue;
        nlohmann::json jc;
        comp->Serialize(jc);
        j["components"].push_back(jc);
    }

    j["scripts"] = nlohmann::json::array();
    for (const auto& script : m_scripts)
    {
        if (!script) continue;
        nlohmann::json js;
        script->Serialize(js);
        j["scripts"].push_back(js);
    }

    j["children"] = nlohmann::json::array();
    for (const auto& child : m_children)
    {
        nlohmann::json cj;
        child.Serialize(cj);
        j["children"].push_back(cj);
    }

    j["hasParent"] = HasParent();
}

void Prefab::Deserialize(const nlohmann::json& j)
{
    if (j.contains("name") && j["name"].is_string())
        m_name = j["name"].get<std::string>();

    if (j.contains("transform")) {
        const auto& jt = j["transform"];
        if (jt.contains("position") && jt["position"].is_array() && jt["position"].size() == 3) {
            auto pos = jt["position"];
            m_transform.SetPosition(XMFLOAT3(pos[0], pos[1], pos[2]));
        }
        if (jt.contains("rotation") && jt["rotation"].is_array() && jt["rotation"].size() == 3) {
            auto rot = jt["rotation"];
            m_transform.SetRotation({ rot[0], rot[1], rot[2] });
        }
        if (jt.contains("scale") && jt["scale"].is_array() && jt["scale"].size() == 3) {
            auto scl = jt["scale"];
            m_transform.SetScale({ scl[0], scl[1], scl[2] });
        }
        if (jt.contains("quat") && jt["quat"].is_array() && jt["quat"].size() == 4) {
            auto q = jt["quat"];
            m_transform.SetQuat({ q[0], q[1], q[2], q[3] });
        }
    }

    if (j.contains("components") && j["components"].is_array()) {
        for (const auto& jc : j["components"]) {
            std::string type = jc.value("type", std::string{});
            if (type.empty()) continue;
			//ToDo : AddComponentByName and Deserialize for the components
        }
    }

    if (j.contains("scripts") && j["scripts"].is_array()) {
        for (const auto& js : j["scripts"]) {
            std::string type = js.value("type", std::string{});
            if (type.empty()) continue;

            //ToDo : AddScriptByName
        }
    }

    if (j.contains("children") && j["children"].is_array()) {
        for (const auto& jc : j["children"]) {
            Prefab child;
            child.Deserialize(std::move(jc));
            AddChild(std::move(child));
        }
    }
}