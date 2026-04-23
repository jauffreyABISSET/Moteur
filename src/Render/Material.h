#pragma once
#include <vector>
#include <d3d12.h>
#include <DirectXMath.h>
#include "DescriptorHeap.h"

class Texture;

class Material
{
public:
    Material();
    ~Material();

    void Initialize();
    void Bind(ID3D12GraphicsCommandList* cmd);
    void SetTexture(Texture* tex);
	void SetTexture(std::string name);
    void SetConstants(const MaterialConstants& data);
    void SetColor(const DirectX::XMFLOAT4& color);
    void SetIsUI(bool value);

    MaterialConstants& GetMaterialConstants();
    bool IsUI();
    ID3D12PipelineState* GetPSO() const;
    ID3D12Resource* GetConstantBuffer();

    bool IsInitialized() const;
    UINT GetTextureCount() const;
    UINT GetDescriptorOffSet() const;
    Texture* GetTexture(size_t index) const;
    std::vector<Texture*>& GetTextures() const;

    std::string GetName() const;

	Material* Clone() const;
private:
    std::string m_name = "Material";
private:
    DescriptorHeap* m_Heap = nullptr;
    ID3D12PipelineState* m_PSO = nullptr;
    ID3D12Resource* m_constantBuffer = nullptr;
    UINT m_cbvDescriptorSize = 0;
    UINT m_DescriptorOffset = UINT_MAX;
    D3D12_GPU_DESCRIPTOR_HANDLE m_GPUDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE{};
    MaterialConstants m_Constants{};
    std::vector<Texture*> m_Textures;

    bool m_initialized = false;
    bool m_isUi = false;
    bool m_lastIsUi = false;

};