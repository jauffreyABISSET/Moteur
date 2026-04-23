#pragma once
#include <string>
#include <memory>

class DescriptorHeap;
class Entity;
class Scene;

class Texture {
public:
    Texture(const std::wstring& path);
    ~Texture();

    static void RegisterTexture(const std::string& key, std::string texturePath);
    static Texture* GetTexture(const std::string& key);

    static std::vector<Entity*> FindOwners(const Texture* tex, Scene* scene);

    Texture* CloneFrom(const Texture* src);

    D3D12_CPU_DESCRIPTOR_HANDLE GetSRV();
    uint32_t GetWidth();
    uint32_t GetHeight();

    const std::string GetName() const;

private:
    ID3D12Resource* m_Resource = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE m_SRV{};
    uint32_t m_Width = 0;
    uint32_t m_Height = 0;

    std::string m_name;

    static std::unordered_map<std::string, std::unique_ptr<Texture>> m_textures;
    void CreateFromFile(const std::wstring& path);
};
