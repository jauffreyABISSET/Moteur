#include "pch.h"
#include "RenderSystem.h"
#include <algorithm>
#include <wincodec.h>
#include "LightManager.h"
#include <dxgidebug.h>

constexpr UINT kMaxObjects = 4096;

RenderSystem& RenderSystem::Get()
{
    static RenderSystem instance;
    return instance;
}

RenderSystem::RenderSystem(ID3D12Device* device,
    ID3D12CommandQueue* queue,
    UINT descriptorCount)
    : m_Device(device)
    , m_Queue(queue)
    , m_Heap()
    , m4xMsaaState(false)
{
    std::fill(std::begin(mFenceValues),
        std::end(mFenceValues),
        0);

    m_Heap.Create(m_Device, descriptorCount);

    // ======================
    // ROOT SIGNATURE
    // ======================
    //
    // Root 0 -> ObjectCB (b0)
    // Root 1 -> Texture table (t0..t15)
    // Root 2 -> MaterialCB (b1)
    // Root 3 -> LightCB (b2)
    //

    CD3DX12_DESCRIPTOR_RANGE texTable;

    texTable.Init(
        D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
        16,
        0);

    CD3DX12_ROOT_PARAMETER rootParams[4];

    rootParams[0].InitAsConstantBufferView(0);

    rootParams[1].InitAsDescriptorTable(
        1,
        &texTable,
        D3D12_SHADER_VISIBILITY_PIXEL);

    rootParams[2].InitAsConstantBufferView(1);

    rootParams[3].InitAsConstantBufferView(2);

    // ======================
    // SAMPLER
    // ======================

    CD3DX12_STATIC_SAMPLER_DESC sampler(
        0,
        D3D12_FILTER_MIN_MAG_MIP_LINEAR,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP
    );

    // ======================
    // ROOT DESC
    // ======================

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc;

    rootSigDesc.Init(
        _countof(rootParams),
        rootParams,
        1,
        &sampler,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    );

    ID3DBlob* serializedRootSig = nullptr;
    ID3DBlob* errorBlob = nullptr;

    ThrowIfFailed(
        D3D12SerializeRootSignature(
            &rootSigDesc,
            D3D_ROOT_SIGNATURE_VERSION_1,
            &serializedRootSig,
            &errorBlob));

    if (errorBlob) {
#if defined(_DEBUG) || defined(DEBUG)
        OutputDebugStringA(
            (char*)errorBlob->GetBufferPointer());
#endif
    }

    ThrowIfFailed(
        device->CreateRootSignature(
            0,
            serializedRootSig->GetBufferPointer(),
            serializedRootSig->GetBufferSize(),
            IID_PPV_ARGS(&m_globalRootSignature)));

    CreateConstantBuffer();
}

bool RenderSystem::Get4xMsaaState()const
{
    return m4xMsaaState;
}

void RenderSystem::Set4xMsaaState(bool value)
{
    if (m4xMsaaState != value)
    {
        m4xMsaaState = value;

        CreateSwapChain();

        if (auto engineApp = &GameManager::GetInstance()->GetAppManager())
        {
            engineApp->OnResize();
        }
    }
}

ID3D12Device* RenderSystem::GetDevice()
{
    return m_Device;
}

void RenderSystem::SetDevice(ID3D12Device* device)
{
    m_Device = device;
}

ID3D12Device* RenderSystem::GetD3DDevice() const
{
    return md3dDevice;
}

void RenderSystem::SetD3DDevice(ID3D12Device* device)
{
    md3dDevice = device;
}

ID3D12CommandQueue* RenderSystem::GetQueue()
{
    return m_Queue;
}

ID3D12CommandQueue* RenderSystem::GetCommandQueue() const
{
    return mCommandQueue;
}

ID3D12CommandAllocator* RenderSystem::GetCommandAllocator() const
{
    return mDirectCmdListAlloc;
}

ID3D12GraphicsCommandList* RenderSystem::GetCommandList() const
{
    return mCommandList;
}
DescriptorHeap* RenderSystem::GetHeap()
{
    return &m_Heap;
}

ID3D12RootSignature* RenderSystem::GetRootSignature()
{
    return m_globalRootSignature;
}

IDXGISwapChain* RenderSystem::GetSwapChain() const
{
    return mSwapChain;
}

void RenderSystem::SetSwapChain(IDXGISwapChain* swapChain)
{
    mSwapChain = swapChain;
}

void RenderSystem::CreateRtvAndDsvDescriptorHeaps()
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
        &rtvHeapDesc, IID_PPV_ARGS(&mRtvHeap)));


    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;
    ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
        &dsvHeapDesc, IID_PPV_ARGS(&mDsvHeap)));

    mRtvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    mDsvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    mCbvSrvUavDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void RenderSystem::CreateCommandObjects()
{
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    ThrowIfFailed(GetD3DDevice()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)));

    ThrowIfFailed(GetD3DDevice()->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(&mDirectCmdListAlloc)));

    ThrowIfFailed(GetD3DDevice()->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        mDirectCmdListAlloc,
        nullptr,
        IID_PPV_ARGS(&mCommandList)));
    mCommandList->Close();

    ThrowIfFailed(GetD3DDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));
    mCurrentFence = 0;

    for (int i = 0; i < SwapChainBufferCount; ++i)
        mFenceValues[i] = mCurrentFence;

#if defined(_DEBUG) || defined(DEBUG)
    OutputDebugStringA("RenderSystem::CreateCommandObjects() - fence created\n");
#endif
}

void RenderSystem::CreateSwapChain()
{
    if (mSwapChain) {
        mSwapChain->SetFullscreenState(false, nullptr);
    }

    if (mdxgiFactory == nullptr) {
#if defined(_DEBUG) || defined(DEBUG)
        OutputDebugStringA("RenderSystem::CreateSwapChain failed: mdxgiFactory is null\n");
#endif
        return;
    }
    if (mCommandQueue == nullptr) {
#if defined(_DEBUG) || defined(DEBUG)
        OutputDebugStringA("RenderSystem::CreateSwapChain failed: mCommandQueue is null\n");
#endif
        return;
    }

    DXGI_SWAP_CHAIN_DESC sd;

    auto& window = GameManager::GetInstance()->GetWindow();
    HWND hwnd = window.GetHandle();
    if (hwnd == nullptr) {
#if defined(_DEBUG) || defined(DEBUG)
        OutputDebugStringA("RenderSystem::CreateSwapChain failed: window handle is null\n");
#endif
        return;
    }

    sd.BufferDesc.Width = window.GetClientWidth();
    sd.BufferDesc.Height = window.GetClientHeight();
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format = mBackBufferFormat;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.SampleDesc.Count = m4xMsaaState ? 2 : 1;
    sd.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = SwapChainBufferCount;
    sd.OutputWindow = hwnd;
    sd.Windowed = true;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    ThrowIfFailed(mdxgiFactory->CreateSwapChain(
        mCommandQueue,
        &sd,
        &mSwapChain));
}

// -----------------------------------------------------------------------------
// New helpers: bounding + frustum helpers
// -----------------------------------------------------------------------------
void RenderSystem::ComputeGeometryBoundingData(Geometry* geo,
    const XMMATRIX& world,
    XMFLOAT3& outWorldCenter,
    float& outApproxRadius,
    XMVECTOR& outAxisX,
    XMVECTOR& outAxisY,
    XMVECTOR& outAxisZ,
    XMFLOAT3& outHalfExtents) const
{
    // Local bounds
    XMFLOAT3 localMin = geo->GetLocalMin();
    XMFLOAT3 localMax = geo->GetLocalMax();

    XMFLOAT3 localCenter{
        (localMin.x + localMax.x) * 0.5f,
        (localMin.y + localMax.y) * 0.5f,
        (localMin.z + localMax.z) * 0.5f
    };

    outHalfExtents = XMFLOAT3(
        (localMax.x - localMin.x) * 0.5f,
        (localMax.y - localMin.y) * 0.5f,
        (localMax.z - localMin.z) * 0.5f
    );

    // approx radius (length of half extents)
    XMVECTOR he = XMLoadFloat3(&outHalfExtents);
    outApproxRadius = XMVectorGetX(XMVector3Length(he));

    // world center position
    XMVECTOR localCenterV = XMLoadFloat3(&localCenter);
    XMVECTOR worldCenterV = XMVector3Transform(localCenterV, world);

    XMStoreFloat3(&outWorldCenter, worldCenterV);

    // world axes (transform normals)
    outAxisX = XMVector3TransformNormal(XMVectorSet(1.f, 0.f, 0.f, 0.f), world);
    outAxisY = XMVector3TransformNormal(XMVectorSet(0.f, 1.f, 0.f, 0.f), world);
    outAxisZ = XMVector3TransformNormal(XMVectorSet(0.f, 0.f, 1.f, 0.f), world);
}

bool RenderSystem::IsEntityVisible(Entity* e, Geometry* geo, const XMMATRIX& world, const XMFLOAT4 planes[6]) const
{
    if (!e || !geo) return false;

    auto boxComp = e->GetComponent<ComponentBoxCollider>();
    auto sphereComp = e->GetComponent<ComponentSphereCollider>();

    if (sphereComp)
    {
        auto pr = sphereComp->ComputeWorldSphere();
        XMFLOAT3 centre = pr.first;
        float radius = pr.second;

        // sphere test
        for (int i = 0; i < 6; ++i) {
            const XMFLOAT4& p = planes[i];
            float dist = p.x * centre.x + p.y * centre.y + p.z * centre.z + p.w;
            if (dist < -radius)
                return false;
        }
        return true;
    }

    if (boxComp)
    {
        auto aabb = boxComp->ComputeWorldAABB();
        const XMFLOAT3& mn = aabb.first;
        const XMFLOAT3& mx = aabb.second;

        for (int i = 0; i < 6; ++i) {
            const XMFLOAT4& pl = planes[i];

            XMFLOAT3 p;
            p.x = (pl.x >= 0.0f) ? mx.x : mn.x;
            p.y = (pl.y >= 0.0f) ? mx.y : mn.y;
            p.z = (pl.z >= 0.0f) ? mx.z : mn.z;
            float dist = pl.x * p.x + pl.y * p.y + pl.z * p.z + pl.w;
            if (dist < 0.0f)
                return false;
        }
        return true;
    }

    // Hybrid approximate OBB test using geometry bounds via helper
    XMFLOAT3 worldCenter;
    float approxRadius = 0.0f;
    XMVECTOR axisX, axisY, axisZ;
    XMFLOAT3 halfExtents;

    ComputeGeometryBoundingData(geo, world, worldCenter, approxRadius, axisX, axisY, axisZ, halfExtents);

    // coarse sphere reject
    for (int p = 0; p < 6; ++p) {
        const XMFLOAT4& pl = planes[p];
        float dist = pl.x * worldCenter.x + pl.y * worldCenter.y + pl.z * worldCenter.z + pl.w;
        if (dist < -approxRadius)
            return false;
    }

    XMVECTOR worldCenterV = XMLoadFloat3(&worldCenter);

    float ex = halfExtents.x;
    float ey = halfExtents.y;
    float ez = halfExtents.z;

    for (int p = 0; p < 6; ++p) {
        const XMFLOAT4& pl = planes[p];
        XMVECTOR n = XMVectorSet(pl.x, pl.y, pl.z, 0.0f);

        XMVECTOR cDot = XMVector3Dot(n, worldCenterV);
        float centerDist = XMVectorGetX(cDot) + pl.w;

        float r =
            fabsf(ex * XMVectorGetX(XMVector3Dot(n, axisX))) +
            fabsf(ey * XMVectorGetX(XMVector3Dot(n, axisY))) +
            fabsf(ez * XMVectorGetX(XMVector3Dot(n, axisZ)));

        if (centerDist + r < 0.0f)
            return false;
    }

    return true;
}

bool RenderSystem::IsVisibleHybrid(Geometry* geo, const XMMATRIX& world, const XMFLOAT4 planes[6]) const
{
    if (!geo)
        return false;

    XMFLOAT3 localMin = geo->GetLocalMin();
    XMFLOAT3 localMax = geo->GetLocalMax();
    XMFLOAT3 localCenter{
        (localMin.x + localMax.x) * 0.5f,
        (localMin.y + localMax.y) * 0.5f,
        (localMin.z + localMax.z) * 0.5f
    };
    XMFLOAT3 halfExtents{
        (localMax.x - localMin.x) * 0.5f,
        (localMax.y - localMin.y) * 0.5f,
        (localMax.z - localMin.z) * 0.5f
    };

    // approximate bounding sphere radius
    XMVECTOR he = XMLoadFloat3(&halfExtents);
    float approxRadius = XMVectorGetX(XMVector3Length(he));

    // world center of geometry
    XMVECTOR localCenterV = XMLoadFloat3(&localCenter);
    XMVECTOR worldCenterV = XMVector3Transform(localCenterV, world);
    XMFLOAT3 worldCenter; XMStoreFloat3(&worldCenter, worldCenterV);

    // quick sphere reject against frustum planes
    for (int p = 0; p < 6; ++p) {
        const XMFLOAT4& pl = planes[p];
        float dist = pl.x * worldCenter.x + pl.y * worldCenter.y + pl.z * worldCenter.z + pl.w;
        if (dist < -approxRadius)
            return false;
    }

    // compute world axes to estimate OBB extents projection
    XMVECTOR axisX = XMVector3TransformNormal(XMVectorSet(1.f, 0.f, 0.f, 0.f), world);
    XMVECTOR axisY = XMVector3TransformNormal(XMVectorSet(0.f, 1.f, 0.f, 0.f), world);
    XMVECTOR axisZ = XMVector3TransformNormal(XMVectorSet(0.f, 0.f, 1.f, 0.f), world);

    float ex = halfExtents.x;
    float ey = halfExtents.y;
    float ez = halfExtents.z;

    for (int p = 0; p < 6; ++p) {
        const XMFLOAT4& pl = planes[p];
        XMVECTOR n = XMVectorSet(pl.x, pl.y, pl.z, 0.0f);

        XMVECTOR cDot = XMVector3Dot(n, worldCenterV);
        float centerDist = XMVectorGetX(cDot) + pl.w;

        float r =
            fabsf(ex * XMVectorGetX(XMVector3Dot(n, axisX))) +
            fabsf(ey * XMVectorGetX(XMVector3Dot(n, axisY))) +
            fabsf(ez * XMVectorGetX(XMVector3Dot(n, axisZ)));

        if (centerDist + r < 0.0f)
            return false;
    }

    return true;
}

bool RenderSystem::ComputeFrustumSphere(const XMFLOAT4 planes[6], const XMFLOAT3& center, float radius) const
{
    for (int i = 0; i < 6; ++i) {
        const XMFLOAT4& p = planes[i];
        float dist = p.x * center.x + p.y * center.y + p.z * center.z + p.w;
        if (dist < -radius)
            return false;
    }
    return true;
}

void RenderSystem::ComputeFrustumPlanes(const XMMATRIX& viewProj, XMFLOAT4 outPlanes[6]) const
{
    XMFLOAT4X4 m;
    XMStoreFloat4x4(&m, viewProj);

    auto makePlane = [&](float a, float b, float c, float d, int idx) {
        XMFLOAT4 p{ a, b, c, d };
        float len = sqrtf(p.x * p.x + p.y * p.y + p.z * p.z);
        outPlanes[idx] = XMFLOAT4(p.x / len, p.y / len, p.z / len, p.w / len);
        };

    // left
    makePlane(m._14 + m._11, m._24 + m._21, m._34 + m._31, m._44 + m._41, 0);
    // right
    makePlane(m._14 - m._11, m._24 - m._21, m._34 - m._31, m._44 - m._41, 1);
    // top
    makePlane(m._14 - m._12, m._24 - m._22, m._34 - m._32, m._44 - m._42, 2);
    // bottom
    makePlane(m._14 + m._12, m._24 + m._22, m._34 + m._32, m._44 + m._42, 3);
    // near
    makePlane(m._14 + m._13, m._24 + m._23, m._34 + m._33, m._44 + m._43, 4);
    // far
    makePlane(m._14 - m._13, m._24 - m._23, m._34 - m._33, m._44 - m._43, 5);
}

void RenderSystem::Render(ID3D12GraphicsCommandList* cmd, Camera* cam)
{
    GameManager* gm = GameManager::GetInstance();

    std::vector<Entity*> entities = gm->GetEntityManager().GetEntities();

    ID3D12DescriptorHeap* heaps[] = { m_Heap.GetHeap() };

    cmd->SetDescriptorHeaps(_countof(heaps), heaps);
    cmd->SetGraphicsRootSignature(m_globalRootSignature);

    // cb contient 3 matrices + un XMFLOAT4 pour gOverrideColor
    UINT cbSize = sizeof(DirectX::XMFLOAT4X4) * 3 + sizeof(DirectX::XMFLOAT4);
    UINT alignedSize = (cbSize + 255) & ~255u;

    XMMATRIX view = cam->GetView();
    XMMATRIX proj = cam->GetProj();
    XMMATRIX viewProj = XMMatrixMultiply(view, proj);

    XMFLOAT4 planes[6];

    // Calculer les plans du frustum avant la boucle
    ComputeFrustumPlanes(viewProj, planes);

    // (optionnel) petit test de sphère centrée sur la caméra
    ComputeFrustumSphere(planes, cam->m_transform.GetLastWorldPosition(), 1.0f);

    size_t idx = 0;

    for (Entity* e : entities)
    {
        if (!e || !e->IsActive())
            continue;

        if (idx >= kMaxObjects) {
            break;
        }

        auto tr = &e->m_transform;
        auto mr = e->GetComponent<ComponentMeshRenderer>();

        if (!tr || !mr) {
#if defined(_DEBUG) || defined(DEBUG)
            OutputDebugStringA("DEBUG:   skip - no transform or no mesh renderer\n");
#endif
            continue;
        }

        Material* mat = mr->GetMaterial();
        Geometry* geo = mr->GetGeometry();

        if (!mat) {
#if defined(_DEBUG) || defined(DEBUG)
            OutputDebugStringA("RenderSystem::Render: skip - material is null\n");
#endif
            continue;
        }

        if (!mat->IsInitialized()) {
            mat->Initialize();
            if (!mat->IsInitialized()) {
#if defined(_DEBUG) || defined(DEBUG)
                OutputDebugStringA("RenderSystem::Render: skip - material failed to initialize\n");
#endif
                continue;
            }
        }

        // Vérification PSO (remise en place)
        if (!mat->GetPSO()) {
#if defined(_DEBUG) || defined(DEBUG)
            OutputDebugStringA("RenderSystem::Render: skip - material PSO is null\n");
#endif
            continue;
        }

        // Assurer que la table SRV est liée (attendue par les materials/shaders)
        CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandleTest{};
        ID3D12DescriptorHeap* srvHeap = m_Heap.GetHeap();
        if (srvHeap)
        {
            gpuHandleTest = CD3DX12_GPU_DESCRIPTOR_HANDLE(srvHeap->GetGPUDescriptorHandleForHeapStart());
        }
        else
        {
            OutputDebugStringA("DBG: srvHeap is null when creating gpuHandleTest\n");
        }
        cmd->SetGraphicsRootDescriptorTable(1, gpuHandleTest);

        // Bind material & geometry
        mat->Bind(cmd);

        if (!geo) {
#if defined(_DEBUG) || defined(DEBUG)
            OutputDebugStringA("RenderSystem::Render: skip - geometry is null\n");
#endif
            continue;
        }
        geo->Bind(cmd);

        XMMATRIX world = tr->GetWorldMatrix();

        bool isUI = mat->IsUI();

        if (!isUI)
        {
            if (!IsEntityVisible(e, geo, world, planes))
            {
#if defined(_DEBUG) || defined(DEBUG)
                OutputDebugStringA("RenderSystem::Render: skip - entity not visible (culled)\n");
#endif
                continue;
            }
        }

        XMMATRIX viewLoc;
        XMMATRIX projLoc;
        if (mat->IsUI())
        {
            viewLoc = XMMatrixIdentity();
            float w = static_cast<float>(GameManager::GetInstance()->GetWindow().GetClientWidth());
            float h = static_cast<float>(GameManager::GetInstance()->GetWindow().GetClientHeight());

            projLoc = XMMatrixOrthographicOffCenterLH(0.0f, w, h, 0.0f, -100.0f, 100.0f);
        }
        else
        {
            viewLoc = cam->GetView();
            projLoc = cam->GetProj();
        }

        XMMATRIX wvp = world * viewLoc * projLoc;

        XMMATRIX worldInvTranspose = XMMatrixInverse(nullptr, world);
        worldInvTranspose = XMMatrixTranspose(worldInvTranspose);

        XMMATRIX wvpT = XMMatrixTranspose(wvp);
        XMMATRIX worldT = XMMatrixTranspose(world);

        DirectX::XMFLOAT4X4 wvpF;
        DirectX::XMFLOAT4X4 worldF;
        DirectX::XMFLOAT4X4 worldInvF;
        XMStoreFloat4x4(&wvpF, wvpT);
        XMStoreFloat4x4(&worldF, worldT);
        XMStoreFloat4x4(&worldInvF, worldInvTranspose);

        UINT64 offset = static_cast<UINT64>(idx) * alignedSize;
        uint8_t* dest = reinterpret_cast<uint8_t*>(m_ObjectCBMappedData) + offset;

        // Offsets calculés explicitement pour éviter erreur d'alignement
        size_t ofs0 = 0;
        size_t ofs1 = ofs0 + sizeof(wvpF);
        size_t ofs2 = ofs1 + sizeof(worldF);
        size_t ofs3 = ofs2 + sizeof(worldInvF);

        // Copier matrices dans le mapped CB
        memcpy(dest + ofs0, &wvpF, sizeof(wvpF));
        memcpy(dest + ofs1, &worldF, sizeof(worldF));
        memcpy(dest + ofs2, &worldInvF, sizeof(worldInvF));

        // --- écrire gOverrideColor attendu par le shader ---
        DirectX::XMFLOAT4 overrideColor = { 0.f, 0.f, 0.f, 0.f };
        if (mr->HasOverrideColor())
        {
            const DirectX::XMFLOAT4& c = mr->GetOverrideColor();
            overrideColor = c;
        }
        memcpy(dest + ofs3, &overrideColor, sizeof(overrideColor));

        // Lier le CB objet et le LightCB
        D3D12_GPU_VIRTUAL_ADDRESS cbAddress = m_ObjectConstantBuffer->GetGPUVirtualAddress() + offset;
        cmd->SetGraphicsRootConstantBufferView(0, cbAddress);
        cmd->SetGraphicsRootConstantBufferView(
            3,
            gm->GetLightManager().GetLightCB()->GetGPUVirtualAddress()
        );

        cmd->DrawIndexedInstanced(geo->GetIndexCount(), 1, 0, 0, 0);

        ++idx;
    }

    // Debug heap start
    if (heaps[0]) {
        D3D12_GPU_DESCRIPTOR_HANDLE gpuStart = heaps[0]->GetGPUDescriptorHandleForHeapStart();
        D3D12_CPU_DESCRIPTOR_HANDLE cpuStart = heaps[0]->GetCPUDescriptorHandleForHeapStart();
#if defined(_DEBUG) || defined(DEBUG)
        wchar_t buf[256];
        swprintf_s(buf, L"DEBUG: RenderSystem::Render - Heap GPUStart=0x%016llX CPUStart=0x%016llX DescSize=%u\n",
            gpuStart.ptr, cpuStart.ptr, RenderSystem::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
        OutputDebugStringW(buf);
#endif
    }
    else {
#if defined(_DEBUG) || defined(DEBUG)
        OutputDebugStringA("DEBUG: RenderSystem::Render - descriptor heap is null\n");
#endif
    }
}

void RenderSystem::CreateConstantBuffer()
{
    ID3D12Device* device = GetDevice();

    // 3 matrices + XMFLOAT4 pour gOverrideColor
    UINT cbSize = sizeof(XMFLOAT4X4) * 3 + sizeof(XMFLOAT4);
    UINT alignedSize = (cbSize + 255) & ~255u;
    UINT64 totalSize = static_cast<UINT64>(alignedSize) * kMaxObjects;

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(totalSize);

    ThrowIfFailed(device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_ObjectConstantBuffer)));

    ThrowIfFailed(m_ObjectConstantBuffer->Map(0, nullptr,
        reinterpret_cast<void**>(&m_ObjectCBMappedData)));
}

ID3D12Resource* RenderSystem::CurrentBackBuffer()const
{
    return mSwapChainBuffer[mCurrBackBuffer];
}

ID3D12Resource* RenderSystem::GetSwapChainBuffer(int index) const
{
    return mSwapChainBuffer[index];
}

void RenderSystem::SetSwapChainBuffer(int index, ID3D12Resource* buffer)
{
    mSwapChainBuffer[index] = buffer;
}

ID3D12DescriptorHeap* RenderSystem::GetRtvHeap() const
{
    return mRtvHeap;
}

ID3D12DescriptorHeap* RenderSystem::GetDsvHeap() const
{
    return mDsvHeap;
}

ID3D12Resource* RenderSystem::GetDepthStencilBuffer() const
{
    return mDepthStencilBuffer;
}

ID3D12Resource* RenderSystem::GetObjectConstantBuffer() const
{
    return m_ObjectConstantBuffer;
}

void RenderSystem::SetDXGIFactory(IDXGIFactory4* factory)
{
    mdxgiFactory = factory;
}

D3D12_VIEWPORT RenderSystem::GetViewport() const
{
    return mScreenViewport;
}

D3D12_RECT RenderSystem::GetScissorRect() const
{
    return mScissorRect;
}

void RenderSystem::SetActiveCamera(Camera* cam)
{
    mActiveCamera = cam;
}

Camera* RenderSystem::GetActiveCamera() const
{
    return mActiveCamera;
}

D3D12_CPU_DESCRIPTOR_HANDLE RenderSystem::CurrentBackBufferView()const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(
        mRtvHeap->GetCPUDescriptorHandleForHeapStart(),
        mCurrBackBuffer,
        mRtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE RenderSystem::DepthStencilView()const
{
    return mDsvHeap->GetCPUDescriptorHandleForHeapStart();
}

void RenderSystem::DrawCommon()
{
    ThrowIfFailed(mDirectCmdListAlloc->Reset());
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc, nullptr));

    mCommandList->SetGraphicsRootSignature(m_globalRootSignature);

    LightManager& lightManager = GameManager::GetInstance()->GetLightManager();

    if (lightManager.GetLightCB())
    {
        mCommandList->SetGraphicsRootConstantBufferView(3, lightManager.GetLightCB()->GetGPUVirtualAddress());
    }

    lightManager.UpdateLightCB();

    auto cmdList = GetCommandList();

    CD3DX12_RESOURCE_BARRIER barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
        CurrentBackBuffer(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    mCommandList->ResourceBarrier(1, &barrier1);

    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScissorRect);

    mCommandList->ClearRenderTargetView(CurrentBackBufferView(), DirectX::Colors::Black, 0, nullptr);
    mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    D3D12_CPU_DESCRIPTOR_HANDLE rtv = CurrentBackBufferView();
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = DepthStencilView();
    mCommandList->OMSetRenderTargets(1, &rtv, true, &dsv);

    Camera* camToUse = mActiveCamera ? mActiveCamera : &m_camera;
    Render(mCommandList, camToUse);
}

void RenderSystem::EndCommonDraw()
{
    CD3DX12_RESOURCE_BARRIER barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
        CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    mCommandList->ResourceBarrier(1, &barrier2);

    ThrowIfFailed(mCommandList->Close());
    ID3D12CommandList* cmdsLists[] = { mCommandList };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    FlushCommandQueue();

    ThrowIfFailed(mSwapChain->Present(0, 0));

    mCurrentFence++;
    ThrowIfFailed(mCommandQueue->Signal(mFence, mCurrentFence));
    mFenceValues[mCurrBackBuffer] = mCurrentFence;

    mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

    FlushCommandQueue();
}

void RenderSystem::FlushCommandQueue()
{
    if (mCommandQueue == nullptr || mFence == nullptr)
    {
#if defined(_DEBUG) || defined(DEBUG)
        OutputDebugStringA("RenderSystem::FlushCommandQueue skipped: mCommandQueue or mFence is null\n");
#endif
        return;
    }

    mCurrentFence++;

    ThrowIfFailed(mCommandQueue->Signal(mFence, mCurrentFence));

    // Wait until the GPU has completed commands up to this fence point.
    if (mFence->GetCompletedValue() < mCurrentFence)
    {
        LPCWSTR name = nullptr;
        HANDLE eventHandle = CreateEventEx(nullptr, name, false, EVENT_ALL_ACCESS);

        ThrowIfFailed(mFence->SetEventOnCompletion(mCurrentFence, eventHandle));

        if (eventHandle != 0) {
            WaitForSingleObject(eventHandle, INFINITE);
            CloseHandle(eventHandle);
        }
    }
}


void RenderSystem::OnResize(UINT width, UINT height)
{
    ID3D12Device* device = GetDevice();

    WaitForFrame(mCurrBackBuffer);

    const UINT bbIndexToWait = mCurrBackBuffer;
    if (mFence->GetCompletedValue() < mFenceValues[bbIndexToWait])
    {
        HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
        ThrowIfFailed(mFence->SetEventOnCompletion(mFenceValues[bbIndexToWait], eventHandle));

        if (eventHandle == 0) {
#if defined(_DEBUG) || defined(DEBUG)
            OutputDebugStringA("RenderSystem::OnResize failed: could not create event handle\n");
#endif
            return;
        }
        else {
            WaitForSingleObject(eventHandle, INFINITE);
            CloseHandle(eventHandle);
        }
    }

    auto cmdList = GetCommandList();
    auto cmdQueue = GetCommandQueue();

    ThrowIfFailed(cmdList->Reset(GetCommandAllocator(), nullptr));

    // Release previous buffers
    for (int i = 0; i < SwapChainBufferCount; ++i) {
        if (mSwapChainBuffer[i]) {
            mSwapChainBuffer[i]->Release();
            mSwapChainBuffer[i] = nullptr;
        }
    }

    if (mDepthStencilBuffer) {
        mDepthStencilBuffer->Release();
        mDepthStencilBuffer = nullptr;
    }

    // Resize swap chain
    ThrowIfFailed(mSwapChain->ResizeBuffers(
        SwapChainBufferCount,
        width, height,
        mBackBufferFormat,
        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

    mCurrBackBuffer = 0;

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < SwapChainBufferCount; ++i) {
        ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainBuffer[i])));
        device->CreateRenderTargetView(mSwapChainBuffer[i], nullptr, rtvHeapHandle);
        rtvHeapHandle.Offset(1, mRtvDescriptorSize);
    }

    D3D12_RESOURCE_DESC depthDesc = {};
    depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.MipLevels = 1;
    depthDesc.DepthOrArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    depthDesc.SampleDesc.Count = m4xMsaaState ? 4u : 1u;
    depthDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
    depthDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE optClear = {};
    optClear.Format = mDepthStencilFormat;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);

    ThrowIfFailed(device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &depthDesc,
        D3D12_RESOURCE_STATE_COMMON,
        &optClear,
        IID_PPV_ARGS(&mDepthStencilBuffer)));

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = mDepthStencilFormat;
    dsvDesc.Texture2D.MipSlice = 0;
    device->CreateDepthStencilView(mDepthStencilBuffer, &dsvDesc, mDsvHeap->GetCPUDescriptorHandleForHeapStart());

    // Transition depth buffer and execute
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        mDepthStencilBuffer,
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_DEPTH_WRITE);
    cmdList->ResourceBarrier(1, &barrier);

    ThrowIfFailed(cmdList->Close());
    ID3D12CommandList* cmds[] = { cmdList };
    cmdQueue->ExecuteCommandLists(_countof(cmds), cmds);

    FlushCommandQueue();

    // Update viewport/scissor owned par RenderSystem
    mScreenViewport.TopLeftX = 0.0f;
    mScreenViewport.TopLeftY = 0.0f;
    mScreenViewport.Width = static_cast<float>(width);
    mScreenViewport.Height = static_cast<float>(height);
    mScreenViewport.MinDepth = 0.0f;
    mScreenViewport.MaxDepth = 1.0f;

    mScissorRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
}

void RenderSystem::WaitForFenceValue(UINT64 value)
{
    if (mFence->GetCompletedValue() < value)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
        ThrowIfFailed(mFence->SetEventOnCompletion(value, eventHandle));

        if (eventHandle == 0) {
#if defined(_DEBUG) || defined(DEBUG)
            OutputDebugStringA("RenderSystem::WaitForFenceValue failed: could not create event handle\n");
#endif
            return;
        }
        else {
            WaitForSingleObject(eventHandle, INFINITE);
            CloseHandle(eventHandle);
        }
    }
}

void RenderSystem::WaitForFrame(UINT frameIndex)
{
    UINT64 fenceValue = mFenceValues[frameIndex];
    WaitForFenceValue(fenceValue);
}

void RenderSystem::InitializeDeviceResources(UINT descriptorCount)
{
    if (m_Device == nullptr)
    {
#if defined(_DEBUG) || defined(DEBUG)
        OutputDebugStringA("RenderSystem::InitializeDeviceResources failed: device is null\n");
#endif
        return;
    }

    if (m_Heap.GetHeap() == nullptr)
    {
        m_Heap.Create(m_Device, descriptorCount);
    }

    // ======================
    //     Root Signature
    // ======================
    CD3DX12_DESCRIPTOR_RANGE texTable;
    texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 16, 0);

    CD3DX12_ROOT_PARAMETER rootParams[4];

    rootParams[0].InitAsConstantBufferView(0);
    rootParams[1].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParams[2].InitAsConstantBufferView(1);
    rootParams[3].InitAsConstantBufferView(2);

    CD3DX12_STATIC_SAMPLER_DESC samplerDesc(
        0,                                    // shaderRegister = s0
        D3D12_FILTER_ANISOTROPIC,             // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,      // addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,      // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP);     // addressW

    CD3DX12_STATIC_SAMPLER_DESC sampler(
        0,
        D3D12_FILTER_MIN_MAG_MIP_LINEAR
    );

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc;
    rootSigDesc.Init(
        _countof(rootParams),
        rootParams,
        0,
        nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    );

    rootSigDesc.Init(
        _countof(rootParams),
        rootParams,
        1,
        &samplerDesc,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    );

    ID3DBlob* serializedRootSig = nullptr;
    ID3DBlob* errorBlob = nullptr;

    HRESULT hr = D3D12SerializeRootSignature(
        &rootSigDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &serializedRootSig,
        &errorBlob);

    if (errorBlob)
    {
#if defined(_DEBUG) || defined(DEBUG)
        OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
#endif
        errorBlob->Release();
        errorBlob = nullptr;
    }
    ThrowIfFailed(hr);

    ThrowIfFailed(m_Device->CreateRootSignature(
        0,
        serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(&m_globalRootSignature)));

    if (serializedRootSig)
    {
        serializedRootSig->Release();
        serializedRootSig = nullptr;
    }

    CreateConstantBuffer();
}

RenderSystem::~RenderSystem()
{
    if (mCommandQueue && mFence)
    {
        mCurrentFence++;
        HRESULT hr = mCommandQueue->Signal(mFence, mCurrentFence);
        if (SUCCEEDED(hr))
        {
            if (mFence->GetCompletedValue() < mCurrentFence)
            {
                HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
                if (eventHandle)
                {
                    mFence->SetEventOnCompletion(mCurrentFence, eventHandle);
                    WaitForSingleObject(eventHandle, INFINITE);
                    CloseHandle(eventHandle);
                }
            }
        }
    }

    for (int i = 0; i < SwapChainBufferCount; ++i)
    {
        if (mSwapChainBuffer[i])
        {
            mSwapChainBuffer[i]->Release();
            mSwapChainBuffer[i] = nullptr;
        }
    }

    if (mDepthStencilBuffer) { mDepthStencilBuffer->Release(); mDepthStencilBuffer = nullptr; }

    if (m_ObjectConstantBuffer) { m_ObjectConstantBuffer->Release(); m_ObjectConstantBuffer = nullptr; }

    if (mCommandList) { mCommandList->Release(); mCommandList = nullptr; }
    if (mDirectCmdListAlloc) { mDirectCmdListAlloc->Release(); mDirectCmdListAlloc = nullptr; }
    if (mCommandQueue) { mCommandQueue->Release(); mCommandQueue = nullptr; }
    if (mFence) { mFence->Release(); mFence = nullptr; }

    if (mRtvHeap) { mRtvHeap->Release(); mRtvHeap = nullptr; }
    if (mDsvHeap) { mDsvHeap->Release(); mDsvHeap = nullptr; }

    if (mSwapChain) { mSwapChain->Release(); mSwapChain = nullptr; }

    if (m_globalRootSignature) { m_globalRootSignature->Release(); m_globalRootSignature = nullptr; }
}