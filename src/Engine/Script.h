#pragma once
#include "GameTimer.h"

class Script
{
protected:
	Entity* m_pEntity = nullptr;
	std::string m_name;

	std::unordered_map<std::string, Script*> m_scripts;

public :
	virtual ~Script() = default;
	virtual void Init() = 0;
	virtual void Update(float dt) = 0;

	virtual std::unique_ptr<Script> Clone() const = 0;
	
	void RegisterScript(const std::string& name, Script* script);

	void SetEntity(Entity* e);
	void Serialize(nlohmann::json& j) const;
	void Deserialize(const nlohmann::json& j);

	std::string GetName() const { return m_name; }

	std::unordered_map<std::string, Script*> GetScripts() const;
};