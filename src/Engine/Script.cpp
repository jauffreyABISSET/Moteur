#include "pch.h"
#include "Script.h"

void Script::RegisterScript(const std::string& name, Script* script)
{
	m_scripts.insert_or_assign(name, script);
}

void Script::SetEntity(Entity* e)
{
	m_pEntity = e;
}

void Script::Serialize(nlohmann::json& j) const
{
	j["type"] = std::to_string(typeid(*this).hash_code());
}

void Script::Deserialize(const nlohmann::json& j)
{

}

std::unordered_map<std::string, Script*> Script::GetScripts() const
{
	return m_scripts;
}