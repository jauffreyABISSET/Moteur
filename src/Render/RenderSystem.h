#pragma once
#include <d3d12.h>
#include "DescriptorHeap.h"
#include "Render/Camera.h"
#include <Core/d3dx12.h>

class Transform;
class ComponentMeshRenderer;

class RenderSystem
{
public:
    // Singleton
    static RenderSystem& Get();

    // Lifecycle
    RenderSystem(ID3D12Device* device, ID3D12CommandQueue* queue, UINT descriptorCount);
    ~RenderSystem();

    // Initialization / resource creation
    void InitializeDeviceResources(UINT descriptorCount);
    void CreateCommandObjects();
    void CreateSwapChain();
    void CreateRtvAndDsvDescriptorHeaps();
    void CreateConstantBuffer();

    // Rendering flow
    void DrawCommon();
    void Render(ID3D12GraphicsCommandList* cmd, Camera* cam);
    void EndCommonDraw();
    void FlushCommandQueue();

    // Resize / synchronization
    void OnResize(UINT width, UINT height);
    void WaitForFenceValue(UINT64 value);
    void WaitForFrame(UINT frameIndex);

    // State / features
    bool Get4xMsaaState() const;
    void Set4xMsaaState(bool value);

    // Accessors / setters
    ID3D12Device* GetDevice();
    void SetDevice(ID3D12Device* device);

    ID3D12Device* GetD3DDevice() const;
    void SetD3DDevice(ID3D12Device* device);

    ID3D12CommandQueue* GetQueue();
    ID3D12CommandQueue* GetCommandQueue() const;

    ID3D12CommandAllocator* GetCommandAllocator() const;
    ID3D12GraphicsCommandList* GetCommandList() const;

    D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
    D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

    DescriptorHeap* GetHeap();
    ID3D12RootSignature* GetRootSignature();

    IDXGISwapChain* GetSwapChain() const;
    void SetSwapChain(IDXGISwapChain* swapChain);

    ID3D12Resource* CurrentBackBuffer() const;
    ID3D12Resource* GetSwapChainBuffer(int index) const;
    void SetSwapChainBuffer(int index, ID3D12Resource* buffer);
    static const int SwapChainBufferCount = 2;

    ID3D12DescriptorHeap* GetRtvHeap() const;
    ID3D12DescriptorHeap* GetDsvHeap() const;

    ID3D12Resource* GetDepthStencilBuffer() const;
    ID3D12Resource* GetObjectConstantBuffer() const;

    void SetDXGIFactory(IDXGIFactory4* factory);

    // Viewport / scissor
    D3D12_VIEWPORT GetViewport() const;
    D3D12_RECT GetScissorRect() const;

    // Camera
    void SetActiveCamera(Camera* cam);
    Camera* GetActiveCamera() const;

private:
    RenderSystem() = default;

    bool IsEntityVisible(class Entity* e, class Geometry* geo, const DirectX::XMMATRIX& world, const DirectX::XMFLOAT4 planes[6]) const;
    bool IsVisibleHybrid(class Geometry* geo, const DirectX::XMMATRIX& world, const DirectX::XMFLOAT4 planes[6]) const;

    void ComputeGeometryBoundingData(class Geometry* geo, const XMMATRIX& world, XMFLOAT3& outWorldCenter, float& outApproxRadius,
        XMVECTOR& outAxisX,
        XMVECTOR& outAxisY,
        XMVECTOR& outAxisZ,
        XMFLOAT3& outHalfExtents) const;

    bool ComputeFrustumSphere(const XMFLOAT4 planes[6], const XMFLOAT3& center, float radius) const;
    void ComputeFrustumPlanes(const XMMATRIX& viewProj, XMFLOAT4 outPlanes[6]) const;
private:
    bool      m4xMsaaState = false;    // 4X MSAA enabled
    UINT      m4xMsaaQuality = 0;      // quality level of 4X MSAA

    IDXGIFactory4* mdxgiFactory = nullptr;
    IDXGISwapChain* mSwapChain = nullptr;
    ID3D12Device* md3dDevice = nullptr;

    ID3D12Device* m_Device = nullptr;
    ID3D12CommandQueue* m_Queue = nullptr;
    DescriptorHeap m_Heap;
    ID3D12RootSignature* m_globalRootSignature = nullptr;

    ID3D12Resource* m_ObjectConstantBuffer = nullptr;
    UINT8* m_ObjectCBMappedData = nullptr;

    ID3D12Fence* mFence = nullptr;
    UINT64 mCurrentFence = 0;

    ID3D12CommandQueue* mCommandQueue = nullptr;
    ID3D12CommandAllocator* mDirectCmdListAlloc = nullptr;
    ID3D12GraphicsCommandList* mCommandList = nullptr;

    int mCurrBackBuffer = 0;
    ID3D12Resource* mSwapChainBuffer[SwapChainBufferCount];
    ID3D12Resource* mDepthStencilBuffer = nullptr;

    ID3D12DescriptorHeap* mRtvHeap = nullptr;
    ID3D12DescriptorHeap* mDsvHeap = nullptr;

    D3D12_VIEWPORT mScreenViewport;
    D3D12_RECT mScissorRect;

    UINT mRtvDescriptorSize = 0;
    UINT mDsvDescriptorSize = 0;
    UINT mCbvSrvUavDescriptorSize = 0;

    D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
    DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    UINT64 mFenceValues[SwapChainBufferCount] = {};

    Camera m_camera;

    Camera* mActiveCamera = nullptr;
};
