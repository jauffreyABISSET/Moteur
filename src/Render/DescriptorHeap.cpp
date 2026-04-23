#include "pch.h"
#include "DescriptorHeap.h"

void DescriptorHeap::Create(ID3D12Device* device, UINT numDescriptors)
{
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = numDescriptors;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    desc.NodeMask = 0;

    device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_Heap));
    m_Size = numDescriptors;
    m_DescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_Current = 0;
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::Allocate()
{
    if (m_Current >= m_Size) return {};

    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_Heap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += m_Current * m_DescriptorSize;
    m_Current++;
    return handle;
}

void DescriptorHeap::CopySRVToHeap(UINT slot, D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, ID3D12Device* device)
{
    if (!m_Heap) return;

    // calcul du handle CPU du slot dans le heap
    D3D12_CPU_DESCRIPTOR_HANDLE dstHandle = m_Heap->GetCPUDescriptorHandleForHeapStart();
    dstHandle.ptr += slot * m_DescriptorSize;

    // copie du SRV dans le slot
    device->CopyDescriptorsSimple(
        1,         // nombre de descriptors à copier
        dstHandle, // destination
        srcHandle, // source
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
    );
}

ID3D12DescriptorHeap* DescriptorHeap::GetHeap()
{
    return m_Heap;
}

UINT DescriptorHeap::GetDescriptorSize()
{
    return m_DescriptorSize;
}

void DescriptorHeap::Release()
{
    if (m_Heap)
    {
        m_Heap->Release();
        m_Heap = nullptr;
    }
}

UINT DescriptorHeap::AllocateBlock(UINT count)
{
    if (m_Current + count > m_Size)
        return UINT_MAX;

    UINT start = m_Current;
    m_Current += count;
    return start;
}