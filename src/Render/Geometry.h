#pragma once
#include <d3d12.h>
#include <stdint.h>
#include <wrl.h>

struct Vertex;

class Geometry
{
public:
    const void Bind(ID3D12GraphicsCommandList* cmd);

    const uint32_t GetIndexCount();

    void DisposeUploaders()
    {
        if (m_vertexUploader) { m_vertexUploader->Release(); m_vertexUploader = nullptr; }
        if (m_indexUploader) { m_indexUploader->Release(); m_indexUploader = nullptr; }
    }

    void SetLocalBounds(const XMFLOAT3& mn, const XMFLOAT3& mx) { m_localMin = mn; m_localMax = mx; }
    XMFLOAT3 GetLocalMin() const { return m_localMin; }
    XMFLOAT3 GetLocalMax() const { return m_localMax; }
    XMFLOAT3 GetLocalSize() const { return XMFLOAT3{ m_localMax.x - m_localMin.x, m_localMax.y - m_localMin.y, m_localMax.z - m_localMin.z }; }

    std::string GetName() const { return m_name; }
private:
    std::string m_name = "Geometry";
public:
    ID3D12Resource* m_vertexBuffer = nullptr;
    ID3D12Resource* m_indexBuffer = nullptr;

    D3D12_VERTEX_BUFFER_VIEW m_VBV{};
    D3D12_INDEX_BUFFER_VIEW  m_IBV{};

    uint32_t m_indexCount = 0;

    ID3D12Resource* m_vertexUploader = nullptr;
    ID3D12Resource* m_indexUploader = nullptr;

    XMFLOAT3 m_localMin{ 0.f, 0.f, 0.f };
    XMFLOAT3 m_localMax{ 0.f, 0.f, 0.f };
};
