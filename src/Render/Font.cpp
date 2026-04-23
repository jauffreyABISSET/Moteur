#include "pch.h"
#include "Font.h"

bool Font::Load(const std::wstring& texturePath,
    int glyphWidth,
    int glyphHeight,
    int columns,
    int rows,
    int firstChar)
{
    m_texture = new Texture(texturePath);

    m_glyphWidth = glyphWidth;
    m_glyphHeight = glyphHeight;

    float texWidth = glyphWidth * columns;
    float texHeight = glyphHeight * rows;

    int charCode = firstChar;

    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < columns; c++)
        {
            float u0 = (c * glyphWidth) / texWidth;
            float v0 = (r * glyphHeight) / texHeight;
            float u1 = ((c + 1) * glyphWidth) / texWidth;
            float v1 = ((r + 1) * glyphHeight) / texHeight;

            m_glyphs[(char)charCode++] = { u0, v0, u1, v1, (float)glyphWidth };
        }
    }

    return true;
}

const Glyph& Font::GetGlyph(char c) const
{
    return m_glyphs.at(c);
}

Texture* Font::GetTexture() const
{
    return m_texture;
}