#include "pch.h"
#include "JsonExporter.h"


using Json = nlohmann::json;

static void DeserializeEntityRecursive(Scene* scene, Entity* entity, const Json& j, bool createChildren = true)
{
    if (j.contains("name")) entity->SetName(j["name"].get<std::string>());
    if (j.contains("active")) entity->SetActive(j["active"].get<bool>());

    Transform* t = &entity->m_transform;
    if (j.contains("position") && j["position"].is_array() && j["position"].size() >= 3) {
        auto p = j["position"];
        XMFLOAT3 pos{ p[0].get<float>(), p[1].get<float>(), p[2].get<float>() };
        t->SetPosition(pos);
    }
    if (j.contains("rotation") && j["rotation"].is_array() && j["rotation"].size() >= 4) {
        auto r = j["rotation"];
        XMFLOAT4 quat{ r[0].get<float>(), r[1].get<float>(), r[2].get<float>(), r[3].get<float>() };
        t->SetQuat(quat);
    }
    if (j.contains("scale") && j["scale"].is_array() && j["scale"].size() >= 3) {
        auto s = j["scale"];
        t->SetScale({ s[0].get<float>(), s[1].get<float>(), s[2].get<float>() });
    }

    if (j.contains("components") && j["components"].is_array()) {
        for (const auto& jc : j["components"]) {
            std::string type = jc.value("type", std::string{});
            if (type.empty()) continue;
            Component* comp = entity->AddComponentByName(type);
            if (comp)
                comp->Deserialize(jc);
        }
    }

    if (j.contains("scripts") && j["scripts"].is_array()) {
        for (const auto& js : j["scripts"]) {
            std::string type = js.value("type", std::string{});
            if (type.empty()) continue;
			entity->AddScriptByName(type);
        }
    }

    // Créer ou mettre à jour les enfants
    if (j.contains("children") && j["children"].is_array()) {
        const auto childrenJson = j["children"];
        if (createChildren) {
            for (const auto& cj : childrenJson) {
                if (!cj.is_object()) continue;
                Entity* child = scene->CreateEntity();
                DeserializeEntityRecursive(scene, child, cj);
                child->SetParent(entity);
                entity->AddChild(child);
            }
        }
        else {
            // utiliser les enfants existants créés par le prefab : recherche par nom prioritaire
            std::vector<Entity*> existingChildren = entity->GetChildren();
            for (size_t i = 0; i < childrenJson.size(); ++i) {
                const auto& cj = childrenJson[i];
                if (!cj.is_object()) continue;

                Entity* targetChild = nullptr;

                // correspondance par nom (si fournie)
                if (cj.contains("name") && cj["name"].is_string()) {
                    std::string childName = cj["name"].get<std::string>();
                    for (Entity* c : existingChildren) {
                        if (c && c->GetName() == childName) {
                            targetChild = c;
                            break;
                        }
                    }
                }

                // fallback : correspondance par index
                if (!targetChild && i < existingChildren.size()) {
                    targetChild = existingChildren[i];
                }

                // si aucun enfant correspondant, en créer un (sécurité)
                if (!targetChild) {
                    targetChild = scene->CreateEntity();
                    targetChild->SetParent(entity);
                    entity->AddChild(targetChild);
                }

                // appliquer la désérialisation sur l'enfant existant
                DeserializeEntityRecursive(scene, targetChild, cj, false);
            }
        }
    }
}

// Recursive initializer: ensure components and scripts are initialized for imported entities and their children
static void InitEntityRecursive(Entity* entity)
{
    if (!entity) return;
    entity->InitComponents();
    entity->InitScript();
    for (Entity* child : entity->GetChildren()) {
        InitEntityRecursive(child);
    }
}

// ----------------------
// SAVE
// ----------------------
void ImportExportJson::SaveSceneToJson(Scene* scene, const std::string& path)
{
    Json root;
    root["scene"] = "GamePlay";

    Json entities = Json::array();

    for (Entity* e : scene->GetEntitiesOfThisScene())
    {
        Json je;
        e->Serialize(je);
        entities.push_back(je);
    }

    root["entities"] = entities;

    std::ofstream file(path);
    file << root.dump(4);
    file.close();
}

// ----------------------
// LOAD
// ----------------------
void ImportExportJson::LoadSceneFromJson(Scene* scene, const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        return;
    }

    Json root;
    try {
        file >> root;
    }
    catch (const nlohmann::json::parse_error& e) {
        return;
    }

    if (!root.is_object() || !root.contains("entities") || !root["entities"].is_array())
    {
        return;
    }

    for (const auto& je : root["entities"])
    {
        if (!je.is_object()) continue;

        int parentIndex = je.value("parent", -1);
        if (parentIndex != -1)
            continue;

        std::string prefab = je.value("prefab", je.value("name", std::string{}));
        if (prefab.empty()) continue;

        XMFLOAT3 pos{ 0.f,0.f,0.f };
        XMFLOAT4 rot{ 0.f,0.f,0.f,1.f };
        if (je.contains("position") && je["position"].is_array() && je["position"].size() >= 3) {
            auto p = je["position"];
            pos = { p[0].get<float>(), p[1].get<float>(), p[2].get<float>() };
        }
        if (je.contains("rotation") && je["rotation"].is_array() && je["rotation"].size() >= 4) {
            auto r = je["rotation"];
            rot = { r[0].get<float>(), r[1].get<float>(), r[2].get<float>(), r[3].get<float>() };
        }

        std::vector<Entity*> created = scene->CreatePrefabs(prefab, pos, rot);

        // DEBUG: tracer ce qui a été demandé / créé
#if defined(_DEBUG) || defined(DEBUG)
        {
            char buf[512];
            sprintf_s(buf, "LoadSceneFromJson: prefab '%s' -> created %zu entities\n", prefab.c_str(), created.size());
            OutputDebugStringA(buf);
        }
#endif

        if (created.empty()) {
            Entity* e = scene->CreateEntity();
            DeserializeEntityRecursive(scene, e, je);
            // Initialize components/scripts so rendering systems (frustum culling, bounds, etc.) are set up
            InitEntityRecursive(e);
            continue;
        }

        Entity* e = created.front();
        DeserializeEntityRecursive(scene, e, je, false);
        // Ensure any added/modified components or children are initialized
        InitEntityRecursive(e);
    }
}

// ----------------------
// ENTITY SERIALIZATION
// ----------------------
Json ImportExportJson::SerializeEntity(const Entity* entity)
{
    Json j;

    j["id"] = entity->GetID();
    j["name"] = entity->GetName();
    j["active"] = entity->IsActive();

    // -------- Transform --------
    const Transform* t = &entity->m_transform;
    Json jt;

    jt["position"] = { t->GetWorldPosition().x, t->GetWorldPosition().y, t->GetWorldPosition().z };
    jt["rotation"] = { t->GetRotation().x, t->GetRotation().y, t->GetRotation().z };
    jt["scale"] = { t->GetScale().x,    t->GetScale().y,    t->GetScale().z };
    jt["quat"] = { t->GetQuat().x,     t->GetQuat().y,     t->GetQuat().z, t->GetQuat().w };

    j["transform"] = jt;

    // -------- Components (base) --------
    Json comps = Json::array();
    for (const auto& c : entity->GetComponents())
    {
        Json jc;
		jc["type"] = std::to_string(typeid(*c).hash_code());
        c->Serialize(jc);
        comps.push_back(jc);
    }
    j["components"] = comps;

    return j;
}

void ImportExportJson::DeserializeEntity(Entity* entity, const Json& j)
{
    entity->SetName(j["name"]);
    entity->SetActive(j["active"]);

    // -------- Transform --------
    Transform* t = &entity->m_transform;

    auto pos = j["transform"]["position"];
    auto rot = j["transform"]["rotation"];
    auto scl = j["transform"]["scale"];
    auto q = j["transform"]["quat"];

	XMFLOAT3 pos3(pos[0], pos[1], pos[2]);
	XMFLOAT4 quat4(q[0], q[1], q[2], q[3]);
    t->SetPosition(pos3);
    t->SetRotation({ rot[0], rot[1], rot[2] });
    t->SetScale({ scl[0], scl[1], scl[2] });
    t->SetQuat(quat4);

    // -------- Components --------
    for (const auto& jc : j["components"])
    {
        std::string type = jc["type"];

        Component* comp = entity->AddComponentByName(type);
        if (comp)
            comp->Deserialize(jc);
    }
}
