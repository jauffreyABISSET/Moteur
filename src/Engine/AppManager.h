#pragma once

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

#pragma comment(lib, "../../ide/Engine/x64/Debug/Engine.lib")

class GamePlayApp;

#include "Render/Window.h"
#include "GameManager.h"
#include "GameTimer.h"

class AppManager
{
    static AppManager* instance;
public:
    static AppManager* GetApp();

    AppManager();
    ~AppManager();

	void Initialize(Window* window);
	void Destroy();

    virtual void Init();
	virtual void Update(const GameTimer& gt);

    void OnResize();

private:
    bool InitDirect3D();
    void CalculateFrameStats(GameTimer gt);

    void LogAdapters();
    void LogAdapterOutputs(IDXGIAdapter* adapter);
    void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

protected:
	Window* m_window = nullptr;
	GameManager* m_gameManager = nullptr;
    GameTimer mTimer;

	GamePlayApp* m_gameApp = nullptr;

    bool      mAppPaused = false;  // is the application paused?
    bool      mMinimized = false;  // is the application minimized?
    bool      mMaximized = false;  // is the application maximized?
    bool      mResizing = false;   // are the resize bars being dragged?
    bool      mFullscreenState = false;// fullscreen enabled

    // Set true to use 4X MSAA (§4.1.8).  The default is false.
    bool      m4xMsaaState = false;    // 4X MSAA enabled
    UINT      m4xMsaaQuality = 0;      // quality level of 4X MSAA

    IDXGIFactory4* mdxgiFactory = nullptr;
    IDXGISwapChain* mSwapChain = nullptr;

    ID3D12Fence* mFence = nullptr;
    UINT64 mCurrentFence = 0;

	D3D12_VIEWPORT mScreenViewport = {};
	D3D12_RECT mScissorRect = {};

    // Derived class should set these in derived constructor to customize starting values.
    D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
    DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
};