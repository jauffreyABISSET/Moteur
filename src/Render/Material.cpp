#include "pch.h"
#include "Material.h"
#include "Texture.h"
#include "DescriptorHeap.h"
#include "RenderSystem.h"
#include "Core/Utils.h"

Material::Material()
{
    m_Constants.color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_Constants.roughness = 0.5f;
    m_Constants.metallic = 0.5f;
}

Material::~Material()
{
    if (m_constantBuffer) {
        m_constantBuffer->Release();
        m_constantBuffer = nullptr;
    }
    if (m_PSO) {
        m_PSO->Release();
        m_PSO = nullptr;
    }
}

// Début de Material::Initialize() : log + gestion du changement de mode UI
void Material::Initialize()
{
    // Debug d'entrée
#if defined(_DEBUG) || defined(DEBUG)
    {
        char buf[256];
        std::snprintf(buf, sizeof(buf),
            "Material::Initialize ENTER: this=%p isUI=%d lastIsUI=%d initialized=%d\n",
            static_cast<void*>(this),
            m_isUi ? 1 : 0,
            m_lastIsUi ? 1 : 0,
            m_initialized ? 1 : 0);
        OutputDebugStringA(buf);
    }
#endif

    if (m_initialized && m_lastIsUi == m_isUi)
        return;

    ID3D12Device* device = RenderSystem::Get().GetDevice();
    if (!device)
    {
        OutputDebugStringA("Material::Initialize failed: D3D device is not ready\n");
        return;
    }

    // Si ré-initialisation demandée, libérer les anciennes ressources
    if (m_PSO) { m_PSO->Release(); m_PSO = nullptr; }
    if (m_constantBuffer) { m_constantBuffer->Release(); m_constantBuffer = nullptr; }

    m_Heap = RenderSystem::Get().GetHeap();
    ID3D12DescriptorHeap* rawHeap = nullptr;
    if (m_Heap)
        rawHeap = m_Heap->GetHeap();

    ID3DBlob* vs = nullptr;
    ID3DBlob* ps = nullptr;
    ID3DBlob* err = nullptr;

    bool failed = false;

    D3D_SHADER_MACRO definesVS[] = { {"VERTEX","1"}, {nullptr,nullptr} };
    HRESULT hr = D3DCompileFromFile(L"../../Shaders/Shader.hlsl", definesVS, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "VS", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vs, &err);
    if (err) { OutputDebugStringA(static_cast<char*>(err->GetBufferPointer())); err->Release(); err = nullptr; }
    if (FAILED(hr))
    {
        std::string msg = "Material::Initialize - VS compile failed HR=" + std::to_string(hr) + "\n";
        OutputDebugStringA(msg.c_str());
        failed = true;
    }

    if (!failed)
    {
        D3D_SHADER_MACRO definesPS[] = { {"PIXEL","1"}, {nullptr,nullptr} };
        hr = D3DCompileFromFile(L"../../Shaders/Shader.hlsl", definesPS, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            "PS", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &ps, &err);
        if (err) { OutputDebugStringA(static_cast<char*>(err->GetBufferPointer())); err->Release(); err = nullptr; }
        if (FAILED(hr))
        {
            std::string msg = "Material::Initialize - PS compile failed HR=" + std::to_string(hr) + "\n";
            OutputDebugStringA(msg.c_str());
            failed = true;
        }
    }

    if (!failed && !RenderSystem::Get().GetRootSignature())
    {
        OutputDebugStringA("Material::Initialize: root signature is null - cannot create PSO\n");
        failed = true;
    }

    if (!failed)
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

        D3D12_INPUT_ELEMENT_DESC inputLayout[] =
        {
        { "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},

        { "COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},

        { "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,28,
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},

        { "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,40,
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0}
        };

        psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
        psoDesc.pRootSignature = RenderSystem::Get().GetRootSignature();
        psoDesc.VS = { vs ? vs->GetBufferPointer() : nullptr, vs ? vs->GetBufferSize() : 0 };
        psoDesc.PS = { ps ? ps->GetBufferPointer() : nullptr, ps ? ps->GetBufferSize() : 0 };
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = TRUE;
        psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
        psoDesc.SampleDesc.Count = 1;

        if (m_isUi)
        {
            // blend
            CD3DX12_BLEND_DESC blendDesc(D3D12_DEFAULT);
            blendDesc.RenderTarget[0].BlendEnable = TRUE;
            blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
            blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
            blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
            blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
            blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

            D3D12_DEPTH_STENCIL_DESC depthDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
            depthDesc.DepthEnable = FALSE;
            depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

            // appliquer les états blend et depth-stencil pour UI
            psoDesc.BlendState = blendDesc;
            psoDesc.DepthStencilState = depthDesc;
        }

        if (FAILED(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PSO))))
        {
            OutputDebugStringA("Material::Initialize: CreateGraphicsPipelineState failed\n");
            failed = true;
        }
    }

    if (!failed)
    {
        // Constant buffer (upload)
        UINT cbSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));
        CD3DX12_HEAP_PROPERTIES heapUpload(D3D12_HEAP_TYPE_UPLOAD);
        CD3DX12_RESOURCE_DESC cbDesc = CD3DX12_RESOURCE_DESC::Buffer(cbSize);

        if (FAILED(device->CreateCommittedResource(
            &heapUpload,
            D3D12_HEAP_FLAG_NONE,
            &cbDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_constantBuffer))))
        {
            OutputDebugStringA("Material::Initialize: CreateCommittedResource for constant buffer failed\n");
            failed = true;
        }
    }

    if (!failed)
    {
        UINT8* mappedData = nullptr;
        D3D12_RANGE readRange = { 0, 0 };
        ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mappedData)));
        memcpy(mappedData, &m_Constants, sizeof(m_Constants));
        m_constantBuffer->Unmap(0, nullptr);

        m_cbvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        if (m_Heap)
        {
            // N'alloue un nouveau bloc que si aucun offset n'a déjà été assigné
            if (m_DescriptorOffset == UINT_MAX)
            {
                m_DescriptorOffset = m_Heap->AllocateBlock(16);
                if (m_DescriptorOffset == UINT_MAX)
                {
                    OutputDebugStringA("Material::Initialize: AllocateBlock failed\n");
                }
            }

            // (Re)calcule le GPU handle à partir du start du heap et de l'offset existant
            if (rawHeap && m_DescriptorOffset != UINT_MAX)
            {
                m_GPUDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(
                    rawHeap->GetGPUDescriptorHandleForHeapStart(),
                    m_DescriptorOffset,
                    m_cbvDescriptorSize);
            }
        }
        else
        {
            m_DescriptorOffset = UINT_MAX;
            OutputDebugStringA("Material::Initialize: DescriptorHeap not available - descriptor-related operations will be skipped\n");
        }

        m_lastIsUi = m_isUi;

        // Succès
        m_initialized = true;
#if defined(_DEBUG) || defined(DEBUG)
        OutputDebugStringA("Material::Initialize: SUCCESS\n");
#endif
    }

    if (vs) vs->Release();
    if (ps) ps->Release();
    if (err) err->Release();
}

void Material::Bind(ID3D12GraphicsCommandList* cmd)
{
    ID3D12RootSignature* globalRoot = RenderSystem::Get().GetRootSignature();
    if (!globalRoot) {
        OutputDebugStringA("Material::Bind: globalRoot is NULL\n");
        return;
    }

    cmd->SetGraphicsRootSignature(globalRoot);

    if (m_PSO) {
        cmd->SetPipelineState(m_PSO);
    }

    // Bind descriptors and log per-slot info
    if (m_Heap)
    {
        ID3D12DescriptorHeap* heaps[] = { m_Heap->GetHeap() };
        cmd->SetDescriptorHeaps(_countof(heaps), heaps);

        UINT descSize = RenderSystem::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(
            m_Heap->GetHeap()->GetGPUDescriptorHandleForHeapStart(),
            m_DescriptorOffset,
            descSize);

        // Log the base GPU handle used for root table
#if defined(_DEBUG) || defined(DEBUG)
        {
            char buf[256];
            std::snprintf(buf, sizeof(buf),
                "Material::Bind: this=%p SetDescriptorHeaps heap=%p descOffset=%u baseGPU=0x%016llx textures=%zu\n",
                static_cast<void*>(this),
                static_cast<void*>(m_Heap->GetHeap()),
                m_DescriptorOffset,
                static_cast<unsigned long long>(gpuHandle.ptr),
                m_Textures.size());
            OutputDebugStringA(buf);
        }
#endif

        // For each texture, compute and log CPU dst, GPU slot and source CPU SRV
        ID3D12DescriptorHeap* rawHeap = m_Heap->GetHeap();
        D3D12_CPU_DESCRIPTOR_HANDLE cpuStart = rawHeap->GetCPUDescriptorHandleForHeapStart();
        D3D12_GPU_DESCRIPTOR_HANDLE gpuStart = rawHeap->GetGPUDescriptorHandleForHeapStart();
        CD3DX12_GPU_DESCRIPTOR_HANDLE gh = gpuHandle;

        for (unsigned i = 0; i < static_cast<unsigned>(m_Textures.size()); ++i)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE srcCpu = m_Textures[i]->GetSRV();
            D3D12_CPU_DESCRIPTOR_HANDLE dstCpu = cpuStart;
            dstCpu.ptr += static_cast<SIZE_T>((m_DescriptorOffset + i) * descSize);

            D3D12_GPU_DESCRIPTOR_HANDLE slotGpu = gpuStart;
            slotGpu.ptr += static_cast<SIZE_T>((m_DescriptorOffset + i) * descSize);

#if defined(_DEBUG) || defined(DEBUG)
            char slotBuf[256];
            std::snprintf(slotBuf, sizeof(slotBuf),
                "Material::Bind: slot=%u srcCPU=0x%016llx dstCPU=0x%016llx slotGPU=0x%016llx\n",
                i,
                static_cast<unsigned long long>(srcCpu.ptr),
                static_cast<unsigned long long>(dstCpu.ptr),
                static_cast<unsigned long long>(slotGpu.ptr));
            OutputDebugStringA(slotBuf);
#endif
        }

        cmd->SetGraphicsRootDescriptorTable(1, gpuHandle);
    }
    else
    {
        OutputDebugStringA("Material::Bind: no heap or invalid descriptor offset\n");
    }

    if (m_constantBuffer)
    {
        auto addr = m_constantBuffer->GetGPUVirtualAddress();
        cmd->SetGraphicsRootConstantBufferView(2, addr);
#if defined(_DEBUG) || defined(DEBUG)
        char cbuf[256];
        std::snprintf(cbuf, sizeof(cbuf), "Material::Bind: CB GPUVA=0x%016llx\n", static_cast<unsigned long long>(addr));
        OutputDebugStringA(cbuf);
#endif

    }
    else
    {
        OutputDebugStringA("Material::Bind: m_constantBuffer is NULL\n");
    }
}

void Material::SetTexture(std::string name)
{
    if (Texture::GetTexture(name) == nullptr)
    {
        std::string msg = "Material::SetTexture - Texture not found: " + name + "\n";
        OutputDebugStringA(msg.c_str());
        return;
    }

    Texture* tex = Texture::GetTexture(name);

    ID3D12Device* device = RenderSystem::Get().GetDevice();

    if (!device) { OutputDebugStringA("Material::SetTexture: device not ready\n"); return; }

    if (!m_Heap) m_Heap = RenderSystem::Get().GetHeap();
    if (!m_Heap) { OutputDebugStringA("Material::SetTexture: heap not ready\n"); return; }

    ID3D12DescriptorHeap* rawHeap = m_Heap->GetHeap();
    if (!rawHeap) { OutputDebugStringA("Material::SetTexture: raw heap null\n"); return; }

    if (m_DescriptorOffset == UINT_MAX)
    {
        m_DescriptorOffset = m_Heap->AllocateBlock(16);
        if (m_DescriptorOffset == UINT_MAX)
        {
            OutputDebugStringA("Material::SetTexture: AllocateBlock failed\n");
            return;
        }

        m_cbvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        m_GPUDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(
            rawHeap->GetGPUDescriptorHandleForHeapStart(),
            m_DescriptorOffset,
            m_cbvDescriptorSize);
    }

    uint32_t slot = static_cast<uint32_t>(m_Textures.size());
    m_Textures.push_back(tex);

    m_Heap->CopySRVToHeap(m_DescriptorOffset + slot, tex->GetSRV(), device);

#if defined(_DEBUG) || defined(DEBUG)
    wchar_t buf[256];
    swprintf_s(buf, L"DBG: Material::SetTexture slot=%u DescriptorOffset=%u SRV.CPU.ptr=0x%p TexturePtr=0x%p\n",
        slot, m_DescriptorOffset, (void*)tex->GetSRV().ptr, (void*)tex);
    OutputDebugStringW(buf);
#endif
}

void Material::SetTexture(Texture* tex)
{
    if (!tex) return;

    ID3D12Device* device = RenderSystem::Get().GetDevice();
    if (!device) return;

    if (!m_Heap)
        m_Heap = RenderSystem::Get().GetHeap();

    if (!m_Heap) return;

    ID3D12DescriptorHeap* rawHeap = m_Heap->GetHeap();
    if (!rawHeap) return;

    UINT descSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    if (m_DescriptorOffset == UINT_MAX)
    {
        m_DescriptorOffset = m_Heap->AllocateBlock(16);
        if (m_DescriptorOffset == UINT_MAX)
        {
            OutputDebugStringA("Material::SetTexture: AllocateBlock failed\n");
            return;
        }
    }

    // Copy SRV into global heap
    D3D12_CPU_DESCRIPTOR_HANDLE src = tex->GetSRV();

    D3D12_CPU_DESCRIPTOR_HANDLE dst = rawHeap->GetCPUDescriptorHandleForHeapStart();
    dst.ptr += static_cast<SIZE_T>(m_DescriptorOffset * descSize);

    device->CopyDescriptorsSimple(
        1,
        dst,
        src,
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    m_Textures.push_back(tex);

    // Debug détaillé : afficher CPU SRC, CPU DST et GPU DST
    {
        CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDst(rawHeap->GetGPUDescriptorHandleForHeapStart(),
            m_DescriptorOffset,
            descSize);

#if defined(_DEBUG) || defined(DEBUG)
        char buf[512];
        std::snprintf(buf, sizeof(buf),
            "Material::SetTexture: this=%p tex=%p srcCPU=0x%016llx dstCPU=0x%016llx dstGPU=0x%016llx descOffset=%u heap=%p\n",
            static_cast<void*>(this),
            static_cast<void*>(tex),
            static_cast<unsigned long long>(src.ptr),
            static_cast<unsigned long long>(dst.ptr),
            static_cast<unsigned long long>(gpuDst.ptr),
            m_DescriptorOffset,
            static_cast<void*>(m_Heap));
        OutputDebugStringA(buf);
#endif
    }
#if defined(_DEBUG) || defined(DEBUG)
    OutputDebugStringA("Material::SetTexture: texture copied into heap\n");
#endif
}

void Material::SetConstants(const MaterialConstants& data)
{
    m_Constants = data;

    // Update upload buffer if created
    if (m_constantBuffer)
    {
        UINT cbSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));
        UINT8* mappedData = nullptr;
        D3D12_RANGE readRange = { 0, 0 };
        ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mappedData)));
        memcpy(mappedData, &m_Constants, sizeof(m_Constants));
        m_constantBuffer->Unmap(0, nullptr);
    }
}

void Material::SetColor(const DirectX::XMFLOAT4& color) {
    m_Constants.color = color;
    SetConstants(m_Constants);
}

void Material::SetIsUI(bool value)
{
    if (m_isUi == value) return;
    m_isUi = value;

    if (m_initialized)
    {
        m_initialized = false;
        Initialize();
    }
}

MaterialConstants& Material::GetMaterialConstants()
{
    return m_Constants;
}

bool Material::IsUI()
{
    return m_isUi;
}

ID3D12PipelineState* Material::GetPSO() const
{
    return m_PSO;
}

ID3D12Resource* Material::GetConstantBuffer()
{
    return m_constantBuffer;
}

bool Material::IsInitialized() const
{
#if defined(_DEBUG) || defined(DEBUG)
    OutputDebugStringA(m_isUi ? "Material::IsInitialized - is UI material\n" : "Material::IsInitialized - is not UI material\n");
#endif

    return m_initialized;
}

UINT Material::GetTextureCount() const
{
    return static_cast<UINT>(m_Textures.size());
}

UINT Material::GetDescriptorOffSet() const
{
    return m_DescriptorOffset;
}

Texture* Material::GetTexture(size_t index) const
{
    if (index < m_Textures.size()) {
        return m_Textures[index];
    }
    return nullptr;
}

std::vector<Texture*>& Material::GetTextures() const
{
    return const_cast<std::vector<Texture*>&>(m_Textures);
}

std::string Material::GetName() const
{
    return m_name;
}

Material* Material::Clone() const
{
    Material* mat = new Material(*this);

    mat->m_initialized = false;
    mat->m_DescriptorOffset = UINT_MAX;
    mat->m_constantBuffer = nullptr;
    mat->m_PSO = nullptr;

    return mat;
}