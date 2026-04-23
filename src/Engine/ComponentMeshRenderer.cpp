#include "pch.h"
#include "ComponentMeshRenderer.h"
#include "Render/Material.h"
#include "Render/Texture.h"
#include "Render/GeometryFactory.h"

ComponentMeshRenderer::ComponentMeshRenderer(Entity* e) : Component(e)
{
    m_pGeo = nullptr;
    m_pMaterial = nullptr;
    m_pTexture = nullptr;
    m_useOverride = false;
}

ComponentMeshRenderer::~ComponentMeshRenderer() {}

void ComponentMeshRenderer::Init()
{
    if (m_pMaterial && !m_pMaterial->IsInitialized())
        m_pMaterial->Initialize();
}

void ComponentMeshRenderer::Update(float dt)
{

}

// --- Geometry ---
void ComponentMeshRenderer::SetGeometry(Geometry* geometry)
{
    if (geometry)
        m_pGeo = geometry;

    if (!m_pMaterial)
        SetMaterial(new Material());
}

Geometry* ComponentMeshRenderer::GetGeometry() const
{
    return m_pGeo;
}

// --- Material ---
void ComponentMeshRenderer::SetMaterial(Material* mat)
{
    if (mat)
        m_pMaterial = mat;

    if (m_pMaterial && m_pTexture)
        m_pMaterial->SetTexture(m_pTexture);
}

Material* ComponentMeshRenderer::GetMaterial() const
{
    return m_pMaterial;
}

// --- Texture ---
void ComponentMeshRenderer::SetTexture(Texture* texture)
{
    m_pTexture = texture;
    if (m_pMaterial && m_pTexture)
        m_pMaterial->SetTexture(m_pTexture);
}

void ComponentMeshRenderer::SetTexture(const std::string& textureName)
{
    m_textureName = textureName;
    m_pTexture = Texture::GetTexture(textureName);
    if (m_pMaterial && m_pTexture)
        m_pMaterial->SetTexture(m_pTexture);
}

Texture* ComponentMeshRenderer::GetTexture() const
{
    return m_pTexture;
}

// --- Override color ---
void ComponentMeshRenderer::SetOverrideColor(const XMFLOAT4& color)
{
    m_overrideColor = color;
    m_useOverride = true;
}

void ComponentMeshRenderer::ClearOverrideColor()
{
    m_useOverride = false;
}

// --- Component interface ---
std::unique_ptr<Component> ComponentMeshRenderer::Clone() const
{
    auto clone = std::make_unique<ComponentMeshRenderer>(m_pEntity);
    clone->m_pGeo = m_pGeo;
    clone->m_pMaterial = m_pMaterial;
    clone->m_pTexture = m_pTexture;
    clone->m_textureName = m_textureName;
    clone->m_overrideColor = m_overrideColor;
    clone->m_useOverride = m_useOverride;
    return clone;
}

void ComponentMeshRenderer::Serialize(nlohmann::json& j) const
{
    j["type"] = "MeshRenderer";
    if (m_pGeo) j["geometry"] = m_pGeo->GetName();
    if (m_pMaterial) j["material"] = m_pMaterial->GetName();
    if (!m_textureName.empty()) j["texture"] = m_textureName;
    if (m_useOverride)
        j["overrideColor"] = { m_overrideColor.x, m_overrideColor.y, m_overrideColor.z, m_overrideColor.w };
}

void ComponentMeshRenderer::Deserialize(const nlohmann::json& j)
{
    if (j.contains("geometry") && j["geometry"].is_string())
    {
        std::string geoName = j["geometry"].get<std::string>();
        m_pGeo = GeometryFactory::GetGeometryByName(geoName);
    }

    if (j.contains("material") && j["material"].is_string())
    {
        std::string matName = j["material"].get<std::string>();
        // TODO: assign material by name si tu as un MaterialFactory
    }

    if (j.contains("texture") && j["texture"].is_string())
    {
        SetTexture(j["texture"].get<std::string>());
    }

    if (j.contains("overrideColor") && j["overrideColor"].is_array() && j["overrideColor"].size() == 4)
    {
        auto c = j["overrideColor"];
        SetOverrideColor({ c[0], c[1], c[2], c[3] });
    }
}

ComponentType* ComponentMeshRenderer::GetTag()
{
    m_type = ComponentType::MeshRenderer;
    return &m_type;
}