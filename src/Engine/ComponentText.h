#pragma once

#include "ComponentMeshRenderer.h"

class Font;

class ComponentText : public ComponentMeshRenderer
{
public:
    ComponentText(Entity* e);
    ~ComponentText();

    bool IsUI() const { return true; }

    void Init() override;
    void Update(float dt) override;

    std::unique_ptr<Component> Clone() const override;
    ComponentType* GetTag() override;

    void SetFont(std::string fontName);
    void SetFont(Font* font);
    void SetText(const std::string& text);
    void SetScale(XMFLOAT2 scale);

private:
    void RebuildMesh();

private:
    Font* m_font = nullptr;
    std::string m_text;
    XMFLOAT2 m_scale = { 1,1 };
};