#include "pch.h"

#include <memory>
#include <vector>

#include "Texture.h"
#include "DescriptorHeap.h"
#include "RenderSystem.h"
#include <wincodec.h>

std::unordered_map<std::string, std::unique_ptr<Texture>> Texture::m_textures;

Texture::~Texture()
{
    if (m_Resource)
    {
        m_Resource->Release();
        m_Resource = nullptr;
    }
}

Texture::Texture(const std::wstring& path)
{
    CreateFromFile(path);
}

void Texture::CreateFromFile(const std::wstring& path)
{
    ID3D12Device* device = RenderSystem::Get().GetDevice();
    ID3D12CommandQueue* queue = RenderSystem::Get().GetCommandQueue();
    DescriptorHeap* heap = RenderSystem::Get().GetHeap();

    if (!device || !queue || !heap)
    {
        OutputDebugStringA("Texture::CreateFromFile: device/queue/heap not ready\n");
        return;
    }

    // --- 1. Load image with WIC ---
    IWICImagingFactory* wicFactory = nullptr;
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&wicFactory));
    if (FAILED(hr) || !wicFactory)
    {
        OutputDebugStringA("Texture::CreateFromFile: WIC factory creation failed\n");
        return;
    }

    IWICBitmapDecoder* decoder = nullptr;
    hr = wicFactory->CreateDecoderFromFilename(
        path.c_str(),
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &decoder);
    if (FAILED(hr) || !decoder)
    {
        OutputDebugStringA("Texture::CreateFromFile: WIC decoder creation failed\n");
        wicFactory->Release();
        return;
    }

    IWICBitmapFrameDecode* frame = nullptr;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr) || !frame)
    {
        OutputDebugStringA("Texture::CreateFromFile: WIC get frame failed\n");
        decoder->Release();
        wicFactory->Release();
        return;
    }

    IWICFormatConverter* converter = nullptr;
    hr = wicFactory->CreateFormatConverter(&converter);
    if (FAILED(hr) || !converter)
    {
        OutputDebugStringA("Texture::CreateFromFile: WIC format converter creation failed\n");
        frame->Release();
        decoder->Release();
        wicFactory->Release();
        return;
    }

    hr = converter->Initialize(
        frame,
        GUID_WICPixelFormat32bppRGBA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0,
        WICBitmapPaletteTypeCustom);
    if (FAILED(hr))
    {
        OutputDebugStringA("Texture::CreateFromFile: WIC converter initialize failed\n");
        converter->Release();
        frame->Release();
        decoder->Release();
        wicFactory->Release();
        return;
    }

    UINT width = 0, height = 0;
    converter->GetSize(&width, &height);
    m_Width = width;
    m_Height = height;

    std::vector<uint8_t> pixels;
    try {
        pixels.resize(static_cast<size_t>(width) * static_cast<size_t>(height) * 4);
    } catch (...) {
        OutputDebugStringA("Texture::CreateFromFile: allocation failed\n");
        converter->Release();
        frame->Release();
        decoder->Release();
        wicFactory->Release();
        return;
    }

    hr = converter->CopyPixels(
        nullptr,
        width * 4,
        static_cast<UINT>(pixels.size()),
        pixels.data());
    if (FAILED(hr))
    {
        OutputDebugStringA("Texture::CreateFromFile: CopyPixels failed\n");
        converter->Release();
        frame->Release();
        decoder->Release();
        wicFactory->Release();
        return;
    }

    // --- 2. Create GPU texture ---
    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    ThrowIfFailed(device->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&m_Resource)));

    // --- 3. Upload buffer ---
    const UINT64 uploadSize = GetRequiredIntermediateSize(m_Resource, 0, 1);

    CD3DX12_HEAP_PROPERTIES heapProp2 = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadSize);

    ID3D12Resource* uploadBuffer = nullptr;
    ThrowIfFailed(device->CreateCommittedResource(
        &heapProp2,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&uploadBuffer)));

    D3D12_SUBRESOURCE_DATA subResourceData = {};
    subResourceData.pData = pixels.data();
    subResourceData.RowPitch = width * 4;
    subResourceData.SlicePitch = subResourceData.RowPitch * height;

    // --- 3b. Create temporary command allocator / command list to perform upload ---
    ID3D12CommandAllocator* uploadAlloc = nullptr;
    ID3D12GraphicsCommandList* uploadCmd = nullptr;
    ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&uploadAlloc)));
    ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, uploadAlloc, nullptr, IID_PPV_ARGS(&uploadCmd)));

    // Record copy + barrier
    UpdateSubresources(uploadCmd, m_Resource, uploadBuffer, 0, 0, 1, &subResourceData);

    CD3DX12_RESOURCE_BARRIER resBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_Resource,
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    uploadCmd->ResourceBarrier(1, &resBarrier);

    ThrowIfFailed(uploadCmd->Close());

    // Execute and wait
    ID3D12CommandList* cmdsLists[] = { uploadCmd };
    queue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Ensure GPU finished upload before creating SRV / releasing upload buffer
    RenderSystem::Get().FlushCommandQueue();

    // --- 4. Create SRV ---
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    DescriptorHeap* heap1 = RenderSystem::Get().GetHeap();
    m_SRV = heap1->Allocate();
    device->CreateShaderResourceView(m_Resource, &srvDesc, m_SRV);

    // DEBUG: log création SRV
#if defined(_DEBUG) || defined(DEBUG)
    {
        wchar_t bufTex[256];
        swprintf_s(bufTex, L"DEBUG: Texture::CreateFromFile path=%s SRV.CPU.ptr=0x%p Resource=0x%p size=%ux%u\n",
                   path.c_str(), (void*)m_SRV.ptr, (void*)m_Resource, m_Width, m_Height);
        OutputDebugStringW(bufTex);
    }
#endif

    // --- 5. Cleanup ---
    if (converter) converter->Release();
    if (frame)     frame->Release();
    if (decoder)   decoder->Release();
    if (wicFactory) wicFactory->Release();
    if (uploadBuffer) uploadBuffer->Release();
    if (uploadCmd) uploadCmd->Release();
    if (uploadAlloc) uploadAlloc->Release();
}

D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetSRV()
{
    return m_SRV;
}

uint32_t Texture::GetWidth()
{
    return m_Width;
}

uint32_t Texture::GetHeight()
{
    return m_Height;
}

void Texture::RegisterTexture(const std::string& key, std::string texturePath)
{
    auto tex = std::make_unique<Texture>(std::wstring(texturePath.begin(), texturePath.end()));

    tex->m_name = key;
    m_textures[key] = std::move(tex);
}

Texture* Texture::GetTexture(const std::string& key)
{
    auto it = m_textures.find(key);
    return (it != m_textures.end()) ? it->second.get() : nullptr;
}

const std::string Texture::GetName() const
{
    return m_name;
}

std::vector<Entity*> Texture::FindOwners(const Texture* tex, Scene* scene)
{
    std::vector<Entity*> owners;
    if (!tex || !scene) return owners;

    auto entities = scene->GetEntitiesOfThisScene();
    for (Entity* e : entities)
    {
        auto mesh = e->GetComponent<ComponentMeshRenderer>();
        if (mesh)
        {
            if (mesh->GetTexture() == tex)
                owners.push_back(e);
        }
    }
    return owners;
}

Texture* Texture::CloneFrom(const Texture* src)
{
    if (!src || !src->m_Resource) return nullptr;

    ID3D12Device* device = RenderSystem::Get().GetDevice();
    ID3D12CommandQueue* queue = RenderSystem::Get().GetCommandQueue();
    DescriptorHeap* heap = RenderSystem::Get().GetHeap();
    if (!device || !queue || !heap) return nullptr;

    D3D12_RESOURCE_DESC desc = src->m_Resource->GetDesc();

    Texture* dst = new Texture(std::wstring());
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
    ThrowIfFailed(device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&dst->m_Resource)));

    ID3D12CommandAllocator* alloc = nullptr;
    ID3D12GraphicsCommandList* cmd = nullptr;
    ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&alloc)));
    ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, alloc, nullptr, IID_PPV_ARGS(&cmd)));

    CD3DX12_RESOURCE_BARRIER barrierSrc = CD3DX12_RESOURCE_BARRIER::Transition(
        src->m_Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE);
    cmd->ResourceBarrier(1, &barrierSrc);

    cmd->CopyResource(dst->m_Resource, src->m_Resource);

    CD3DX12_RESOURCE_BARRIER barrierSrcBack = CD3DX12_RESOURCE_BARRIER::Transition(
        src->m_Resource, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    cmd->ResourceBarrier(1, &barrierSrcBack);

    ThrowIfFailed(cmd->Close());
    ID3D12CommandList* lists[] = { cmd };
    queue->ExecuteCommandLists(1, lists);
    RenderSystem::Get().FlushCommandQueue();

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;

    dst->m_SRV = RenderSystem::Get().GetHeap()->Allocate();
    device->CreateShaderResourceView(dst->m_Resource, &srvDesc, dst->m_SRV);

    if (cmd) cmd->Release();
    if (alloc) alloc->Release();

    return dst;
}
