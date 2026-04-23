#pragma once
#include "pch.h"
#include "Component.h"

class ComponentLight : public Component
{
public:
    ComponentLight(Entity* e = nullptr);
    virtual ~ComponentLight();

    void Init() override;
    //ComponentLight& DefaultPointLight();
    //ComponentLight& DefaultSpotLight();
    void Update(float dt) override;
    void UseEntityForward(bool value);
    void SetType(LightType type);
    void SetColor(XMFLOAT4 color);
    void SetColor(float r, float g, float b, float a);
    void SetDirection(float x, float y, float z);
    void SetIntensity(float i);
    void SetPosition(float x, float y, float z);
    void SetStrength(float strength);
    void SetRange(float range);
    void SetAngle(float angle);
    void SetPenumbra(float outer);
    void SetRimLightColor(float r, float g, float b, float a);
    void SetRimLightIntensity(float intensity);

    void SetEntity(Entity* e);

    LightData& GetData();

    std::unique_ptr<Component> Clone() const override;
    ComponentType* GetTag() override;

	void Serialize(nlohmann::json& j) const override;
	void Deserialize(const nlohmann::json& j) override;

    std::string GetName() const { return m_name; }
private:
    std::string m_name = "LightComponent";
private:

    LightData mLight;
    bool m_useEntityForward = true;
    LightManager* lm = nullptr;
};