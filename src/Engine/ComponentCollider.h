#pragma once
#include "Component.h"

class ComponentCollider : public Component
{
	bool ignoreFirstFrame = true;
public:
	ComponentCollider(Entity* e);
	~ComponentCollider() override;

	void Init() override;
	void Update(float dt) override;
	void OnCollision();
	bool Intersects(const Entity* other) const;

	std::unique_ptr<Component> Clone() const override;

	void Serialize(nlohmann::json& j) const override;
	void Deserialize(const nlohmann::json& j) override;

	ComponentType* GetTag() override;
};
