#pragma once

#include <unordered_map>
#include <string>

class Font;

class FontManager
{
public:
    static FontManager& Get();

    static void RegisterFont(const std::string& name, std::string path, int rows = 16, int columns = 16, int totalPixelW = 512, int totalPixelH = 512, int glyphW = 32, int glyphH = 32);

    static Font* LoadFont(const std::string& name, const std::wstring& texturePath,int glyphW, int glyphH, int columns, int rows);

    Font* GetFont(const std::string& name);

private:
    static std::unordered_map<std::string, std::unique_ptr<Font>> m_fonts;

};