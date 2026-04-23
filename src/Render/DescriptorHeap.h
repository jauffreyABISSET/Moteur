#pragma once
#include <d3d12.h>

class DescriptorHeap
{
private:
    ID3D12DescriptorHeap* m_Heap = nullptr;
    UINT m_Size = 0;
    UINT m_DescriptorSize = 0;
    UINT m_Current = 0;

public:
    DescriptorHeap() = default;

    void Create(ID3D12Device* device, UINT numDescriptors);

    D3D12_CPU_DESCRIPTOR_HANDLE Allocate();

    void CopySRVToHeap(UINT slot, D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, ID3D12Device* device);

    ID3D12DescriptorHeap* GetHeap();
    UINT GetDescriptorSize();

    void Release();

    UINT AllocateBlock(UINT count);
};