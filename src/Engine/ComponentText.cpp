#include "pch.h"
#include "ComponentText.h"
#include <cstdio>

ComponentText::ComponentText(Entity* e) : ComponentMeshRenderer(e)
{
}

ComponentText::~ComponentText()
{
}

void ComponentText::Init()
{
    Material* mat = GetMaterial();
    bool created = false;
    if (!mat)
    {
        mat = new Material();
        created = true;
    }

    if (mat)
    {
        mat->SetIsUI(true);
        mat->Initialize();
    }

    MaterialConstants constants = {};
    constants.color = { 1,1,1,1 };
    constants.roughness = 0;
    constants.metallic = 0;
    constants.isUI = 1.0f;

    mat->SetConstants(constants);
    if (created)
        SetMaterial(mat);

    if (m_pEntity)
    {
        XMFLOAT3 pos = m_pEntity->m_transform.GetLocalPosition();
        if (pos.z != 0.0f)
        {
            pos.z = 0.0f;
            m_pEntity->m_transform.SetPosition(pos);
            OutputDebugStringA("ComponentText::Init: forced Z=0 on text entity\n");
        }
    }
}

void ComponentText::Update(float dt)
{

}

std::unique_ptr<Component> ComponentText::Clone() const
{
    return std::make_unique<ComponentText>(*this);
}

ComponentType* ComponentText::GetTag()
{
    m_type = ComponentType::Text;
    return &m_type;
}

void ComponentText::SetFont(std::string fontName)
{
#if defined(_DEBUG) || defined(DEBUG)
    std::string msg = "ComponentText::SetFont requested name='" + fontName + "'\n";
    OutputDebugStringA(msg.c_str());
#endif

    Font* font = FontManager::Get().GetFont(fontName);
    if (!font)
    {
        std::string err = "ComponentText::SetFont - FontManager returned NULL for '" + fontName + "'\n";
        OutputDebugStringA(err.c_str());
        return;
    }

    SetFont(font);
}

void ComponentText::SetFont(Font* font)
{
    m_font = font;
    if (!font) return;

    Texture* tex = font->GetTexture();
    if (tex)
    {
        SetTexture(tex);

#if defined(_DEBUG) || defined(DEBUG)
        wchar_t buf[256];
        swprintf_s(buf, L"ComponentText::SetFont: assigned texture ptr=0x%p\n", (void*)tex);
        OutputDebugStringW(buf);
#endif
    }
    else
    {
        OutputDebugStringA("ComponentText::SetFont: font->GetTexture() returned NULL\n");
    }
}

void ComponentText::SetText(const std::string& text)
{
    m_text = text;
    RebuildMesh();
}

void ComponentText::SetScale(XMFLOAT2 scale)
{
    m_scale = scale;
    RebuildMesh();
}

void ComponentText::RebuildMesh()
{
    if (!m_font) return;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    float cursorX = 0.f;
    float cursorY = 0.f;

    for (char c : m_text)
    {
        if (c == '\n')
        {
            cursorY -= m_scale.x;
            cursorX = 0;
            continue;
        }

        const Glyph& g = m_font->GetGlyph(c);

        float w = m_scale.x;
        float h = m_scale.y;

        uint32_t start = vertices.size();

        // top-left
        vertices.push_back({ {cursorX, cursorY, 0}, {1,1,1,1}, {0,0,1}, {g.u0, g.v0} });
        // top-right
        vertices.push_back({ {cursorX + w, cursorY, 0}, {1,1,1,1}, {0,0,1}, {g.u1, g.v0} });
        // bottom-left
        vertices.push_back({ {cursorX, cursorY + h, 0}, {1,1,1,1}, {0,0,1}, {g.u0, g.v1} });
        // bottom-right
        vertices.push_back({ {cursorX + w, cursorY + h, 0}, {1,1,1,1}, {0,0,1}, {g.u1, g.v1} });

        indices.insert(indices.end(),
            {
                start, start + 1, start + 2,
                start + 1, start + 3, start + 2
            });

        cursorX += w;
    }

    if (vertices.empty() || indices.empty())
    {
        SetGeometry(nullptr);
        return;
    }

    Geometry* geo = GeometryFactory::CreateCustom(
        vertices.data(),
        vertices.size(),
        indices.data(),
        indices.size(),
        false
    );

    SetGeometry(geo);
}