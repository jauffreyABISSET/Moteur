#include "pch.h"
#include "FontManager.h"

std::unordered_map<std::string, std::unique_ptr<Font>> FontManager::m_fonts;

FontManager& FontManager::Get()
{
    static FontManager instance;
    return instance;
}

void FontManager::RegisterFont(const std::string& name, std::string path, 
    int rows, int columns, 
    int totalPixelW, int totalPixelH, 
    int glyphW, int glyphH)
{
    auto font = std::make_unique<Font>();
    std::wstring wpath(path.begin(), path.end());

    if (glyphW == 32 && totalPixelW != 512)
        glyphW = totalPixelW / columns;

    if (glyphH == 32 && totalPixelH != 512)
        glyphH = totalPixelH / rows;

    font->Load(wpath, glyphW, glyphH, columns, rows);

    m_fonts[name] = std::move(font);
}

Font* FontManager::LoadFont(const std::string& name, const std::wstring& texturePath,
    int glyphW, int glyphH, int columns, int rows)
{
    auto font = std::make_unique<Font>();
    font->Load(texturePath, glyphW, glyphH, columns, rows);

    Font* fontPtr = font.get();
    m_fonts[name] = std::move(font);
    return fontPtr;
}

Font* FontManager::GetFont(const std::string& name)
{
    auto it = m_fonts.find(name);

    if (it != m_fonts.end())
        return it->second.get();
    return nullptr;
}