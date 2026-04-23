#include "pch.h"
#include "ComponentLight.h"
#include "GameManager.h"
#include "Render/LightManager.h"
#include <Windows.h>

ComponentLight::ComponentLight(Entity* e)
    : Component(e)
    , lm(nullptr)
    , mLight{}
{
    mLight.type = LightType::POINT;
    m_pEntity = e;
}

ComponentLight::~ComponentLight()
{
    if (lm)
    {
        lm->UnregisterLight(&mLight);
    }
}

void ComponentLight::Init()
{
    lm = &GameManager::GetInstance()->GetLightManager();

    if (m_pEntity)
    {
        mLight.position = m_pEntity->m_transform.GetWorldPosition();
        mLight.direction = { 0.f, 0.f, 0.f };
        mLight.color = { 1.f, 1.f, 1.f, 1.f };
        mLight.range = 10.f;
        mLight.intensity = 1.f;
        mLight.strength = { 1.f,1.f,1.f };
        mLight.rimLightColor = { 1.f,1.f,1.f,1.f };
        mLight.rimLightIntensity = 0.f;
        mLight.spotAngle = 30.f;
        mLight.penumbraAngle = 45.f;
        if (mLight.type == LightType::NONE)
            mLight.type = LightType::POINT;
    }

    if (lm)
    {
        bool ok = lm->RegisterLight(&mLight);
        if (!ok) {
            OutputDebugStringA("ComponentLight::Init - RegisterLight FAILED\n");
        } else {
#if defined(_DEBUG) || defined(DEBUG)
            char buf[128];
            sprintf_s(buf, "ComponentLight::Init - registered light id=%d\n", mLight.id);
            OutputDebugStringA(buf);
#endif
        }
    }
}

void ComponentLight::UseEntityForward(bool value)
{
    m_useEntityForward = value;
}

void ComponentLight::Update(float dt)
{
    if (!m_pEntity) return;

    mLight.position = m_pEntity->m_transform.GetWorldPosition();

    if (mLight.type == LightType::SPOT && m_useEntityForward)
    {
        XMMATRIX world = m_pEntity->m_transform.GetWorldMatrix();
        XMVECTOR forward = XMVector3TransformNormal(
            XMVectorSet(0.f, 0.f, 1.f, 0.f), world);

        forward = XMVector3Normalize(forward);
        XMStoreFloat3(&mLight.direction, forward);
    }
}

LightData& ComponentLight::GetData()
{
    return mLight;
}

void ComponentLight::SetType(LightType type) { mLight.type = type; }
void ComponentLight::SetColor(XMFLOAT4 color) { mLight.color = color; }
void ComponentLight::SetColor(float r, float g, float b, float a) { mLight.color = { r,g,b,a }; }
void ComponentLight::SetDirection(float x, float y, float z) { mLight.direction = { x,y,z }; }
void ComponentLight::SetIntensity(float i) { mLight.intensity = i; }
void ComponentLight::SetPosition(float x, float y, float z) { mLight.position = { x,y,z }; }
void ComponentLight::SetStrength(float s) { mLight.strength = { s,s,s }; }
void ComponentLight::SetRange(float r) { mLight.range = r; }
void ComponentLight::SetAngle(float inner) { mLight.spotAngle = inner; }
void ComponentLight::SetPenumbra(float outer) { mLight.penumbraAngle = outer; }
void ComponentLight::SetRimLightColor(float r, float g, float b, float a) { mLight.rimLightColor = { r,g,b,a }; }
void ComponentLight::SetRimLightIntensity(float i) { mLight.rimLightIntensity = i; }
void ComponentLight::SetEntity(Entity* e) { m_pEntity = e; }

ComponentType* ComponentLight::GetTag() { return new ComponentType(ComponentType::Light); }

void ComponentLight::Serialize(nlohmann::json& j) const
{
    j["type"] = "Light";
    j["color"] = { mLight.color.x, mLight.color.y, mLight.color.z, mLight.color.w };
    j["position"] = { mLight.position.x, mLight.position.y, mLight.position.z };
    j["direction"] = { mLight.direction.x, mLight.direction.y, mLight.direction.z };
    j["intensity"] = mLight.intensity;
    j["range"] = mLight.range;
    j["strength"] = { mLight.strength.x, mLight.strength.y, mLight.strength.z };
    j["rimLightColor"] = { mLight.rimLightColor.x, mLight.rimLightColor.y, mLight.rimLightColor.z, mLight.rimLightColor.w };
	j["rimLightIntensity"] = mLight.rimLightIntensity;
}

void ComponentLight::Deserialize(const nlohmann::json& j)
{
    if (j.contains("color") && j["color"].is_array() && j["color"].size() == 4) {
        mLight.color = { j["color"][0].get<float>(), j["color"][1].get<float>(), j["color"][2].get<float>(), j["color"][3].get<float>() };
    }
    if (j.contains("position") && j["position"].is_array() && j["position"].size() == 3) {
        mLight.position = { j["position"][0].get<float>(), j["position"][1].get<float>(), j["position"][2].get<float>() };
    }
    if (j.contains("direction") && j["direction"].is_array() && j["direction"].size() == 3) {
        mLight.direction = { j["direction"][0].get<float>(), j["direction"][1].get<float>(), j["direction"][2].get<float>() };
    }
    if (j.contains("intensity")) {
        mLight.intensity = j["intensity"].get<float>();
    }
    if (j.contains("range")) {
        mLight.range = j["range"].get<float>();
    }
    if (j.contains("strength") && j["strength"].is_array() && j["strength"].size() == 3) {
        mLight.strength = { j["strength"][0].get<float>(), j["strength"][1].get<float>(), j["strength"][2].get<float>() };
    }
    if (j.contains("rimLightColor") && j["rimLightColor"].is_array() && j["rimLightColor"].size() == 4) {
        mLight.rimLightColor = { j["rimLightColor"][0].get<float>(), j["rimLightColor"][1].get<float>(), j["rimLightColor"][2].get<float>(), j["rimLightColor"][3].get<float>() };
    }
    if (j.contains("rimLightIntensity")) {
        mLight.rimLightIntensity = j["rimLightIntensity"].get<float>();
	}
}

std::unique_ptr<Component> ComponentLight::Clone() const { return std::make_unique<ComponentLight>(*this); }

