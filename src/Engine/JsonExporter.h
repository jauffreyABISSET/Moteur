#pragma once
#include <nlohmann/json.hpp>

using Json = nlohmann::json;

class Scene;

class ImportExportJson
{
public:
    void SaveSceneToJson(Scene* scene, const std::string& path);
    void LoadSceneFromJson(Scene* scene, const std::string& path);

private:
    static Json SerializeEntity(const Entity* entity);
    static void DeserializeEntity(Entity* entity, const Json& j);
};

