#pragma once
#include "Component.h"
#include "Render/Material.h"
#include "Render/Geometry.h"
#include "Render/GeometryFactory.h"
#include "Render/Texture.h"
#include "Engine/JsonExporter.h"
#include <memory>
#include <string>

class Entity;
class Geometry;
class Material;
class Texture;

class ComponentMeshRenderer : public Component
{
    Geometry* m_pGeo = nullptr;
    Material* m_pMaterial = nullptr;
    Texture* m_pTexture = nullptr;
    std::string m_textureName;

    XMFLOAT4 m_overrideColor = { 0,0,0,0 };
    bool m_useOverride = false;

public:
    ComponentMeshRenderer(Entity* e);
    ~ComponentMeshRenderer() override;

    void Init() override;
    void Update(float dt) override;

    void SetGeometry(Geometry* geometry = GeometryFactory::CreateCube());
    Geometry* GetGeometry() const;

    void SetMaterial(Material* mat = new Material());
    Material* GetMaterial() const;

    void SetTexture(Texture* tex);
    void SetTexture(const std::string& textureName);
    Texture* GetTexture() const;
    const std::string& GetTextureName() const { return m_textureName; }

    void SetOverrideColor(const XMFLOAT4& color);
    void ClearOverrideColor();
    bool HasOverrideColor() const { return m_useOverride; }
    const XMFLOAT4& GetOverrideColor() const { return m_overrideColor; }

    std::unique_ptr<Component> Clone() const override;
    void Serialize(nlohmann::json& j) const override;
    void Deserialize(const nlohmann::json& j) override;

    ComponentType* GetTag() override;
    std::string GetTypeName() const { return "MeshRenderer"; }

    Entity* GetEntity() const { return m_pEntity; }
};