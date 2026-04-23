#pragma once
#include <unordered_map>
#include <string>

class Texture;

struct Glyph
{
    float u0, v0;
    float u1, v1;
    float width;
};

class Font
{
public:
    Font() = default;
    ~Font() = default;

    bool Load(const std::wstring& texturePath,
        int glyphWidth,
        int glyphHeight,
        int columns,
        int rows,
        int firstChar = 32);

    const Glyph& GetGlyph(char c) const;
    Texture* GetTexture() const;

private:
    std::unordered_map<char, Glyph> m_glyphs;
    Texture* m_texture = nullptr;

    int m_glyphWidth = 0;
    int m_glyphHeight = 0;
};