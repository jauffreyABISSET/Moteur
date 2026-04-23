#include "pch.h"
#include "AppManager.h"
#include "GameManager.h"
#include "Gameplay/GamePlayApp.h"

using namespace DirectX;

AppManager* AppManager::instance = nullptr;

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

AppManager* AppManager::GetApp()
{
	if (instance == nullptr) {
		instance = new AppManager();
	}
	return instance;
}

void AppManager::Destroy()
{
	//DestroyDebugConsole();
	delete instance;
	instance = nullptr;
}

void AppManager::Initialize(Window* window)
{
	//CreateDebugConsole();
	m_window = window;
	InitDirect3D();
}

void AppManager::Init()
{
	m_gameApp = new GamePlayApp(*m_window);
	m_gameApp->Init();
}

void AppManager::Update(const GameTimer& gt)
{
	if (!mAppPaused)
	{
		if (m_gameApp)
			m_gameApp->Update(gt);
	}

	CalculateFrameStats(gt);
}

AppManager::AppManager() : mAppPaused(false), m_window(nullptr), mdxgiFactory(nullptr), m4xMsaaQuality(0)
{

}

AppManager::~AppManager()
{

}

void AppManager::CalculateFrameStats(GameTimer gt)
{
	static int frameCnt = 0;
	static double timeElapsed = 0.0;
	frameCnt++;

	double totalTime = gt.TotalTime();

	if ((totalTime - timeElapsed) >= 1.0)
	{
		float fps = static_cast<float>(frameCnt);
		float mspf = (fps > 0.0f) ? (1000.0f / fps) : 0.0f;
		std::wostringstream outs;
		outs.precision(4);
		outs << L"FPS: " << fps << L"    "
			<< L"Frame Time: " << mspf << L" (ms)";

		if (m_window)
			SetWindowText(m_window->GetHandle(), outs.str().c_str());

		frameCnt = 0;
		timeElapsed = totalTime;
	}
}

void AppManager::OnResize()
{
	RenderSystem& render = RenderSystem::Get();

	assert(render.GetD3DDevice());

	render.FlushCommandQueue();

	UINT w = m_window->GetClientWidth();
	UINT h = m_window->GetClientHeight();

	render.OnResize(w, h);

	mScreenViewport = render.GetViewport();
	mScissorRect = render.GetScissorRect();
}

bool AppManager::InitDirect3D()
{
#if defined(DEBUG) || defined(_DEBUG)
    {
        ID3D12Debug* debugController = nullptr;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();
            debugController->Release();
        }
        else
        {
            OutputDebugStringA("DEBUG: D3D12GetDebugInterface failed - debug layer not enabled\n");
        }
    }
#endif
	RenderSystem& render = RenderSystem::Get();

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&mdxgiFactory)));
	render.SetDXGIFactory(mdxgiFactory);

	// Try to create hardware device.
	ID3D12Device* device = nullptr;
	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr,             // default adapter
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&device));

	// Fallback to WARP device.
	if (FAILED(hardwareResult))
	{
		IDXGIAdapter* pWarpAdapter = nullptr;
		ThrowIfFailed(mdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			pWarpAdapter,
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&device)));

		ReleaseCom(pWarpAdapter);
	}
	render.SetD3DDevice(device);
	render.SetDevice(device);

	// Create RenderSystem command objects (command-queue, allocators, list, fence) before swapchain creation.
	// RenderSystem is responsible for its own fence/command-queue; avoid creating a separate AppManager fence here.
	render.CreateCommandObjects();

	{
		auto heap = render.GetHeap();
		if (heap)
		{
			const UINT descriptorCount = 256;
			render.InitializeDeviceResources(descriptorCount);
		}
		else
		{
			OutputDebugStringA("EngineApp::InitDirect3D: RenderSystem::Get().GetHeap() returned nullptr\n");
		}
	}

	// No local mFence creation here: RenderSystem::CreateCommandObjects() must create the fence used by FlushCommandQueue.

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = mBackBufferFormat;
    msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	ThrowIfFailed(render.GetD3DDevice()->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)));

	m4xMsaaQuality = msQualityLevels.NumQualityLevels;
	assert(m4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

#ifdef _DEBUG
	LogAdapters();
#endif

	// Create swap chain and RTV/DSV heaps after the command queue exists.
	render.CreateSwapChain();
	render.CreateRtvAndDsvDescriptorHeaps();

	if (m_window)
	{
		UINT w = m_window->GetClientWidth();
		UINT h = m_window->GetClientHeight();
		render.OnResize(w, h);
	}

	return true;
}

void AppManager::LogAdapters()
{
	UINT i = 0;
	IDXGIAdapter* adapter = nullptr;
	std::vector<IDXGIAdapter*> adapterList;
	while (mdxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC desc;
		adapter->GetDesc(&desc);

		std::wstring text = L"***Adapter: ";
		text += desc.Description;
		text += L"\n";

		OutputDebugString(text.c_str());

		adapterList.push_back(adapter);

		++i;
	}

	for (size_t i = 0; i < adapterList.size(); ++i)
	{
		LogAdapterOutputs(adapterList[i]);
		ReleaseCom(adapterList[i]);
	}
}

void AppManager::LogAdapterOutputs(IDXGIAdapter* adapter)
{
	UINT i = 0;
	IDXGIOutput* output = nullptr;
	while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_OUTPUT_DESC desc;
		output->GetDesc(&desc);

		std::wstring text = L"***Output: ";
		text += desc.DeviceName;
		text += L"\n";
		OutputDebugString(text.c_str());

		//LogOutputDisplayModes(output, mBackBufferFormat);

		ReleaseCom(output);

		++i;
	}
}

void AppManager::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
{
	UINT count = 0;
	UINT flags = 0;

	// Call with nullptr to get list count.
	output->GetDisplayModeList(format, flags, &count, nullptr);

	std::vector<DXGI_MODE_DESC> modeList(count);
	output->GetDisplayModeList(format, flags, &count, &modeList[0]);

	for (auto& x : modeList)
	{
		UINT n = x.RefreshRate.Numerator;
		UINT d = x.RefreshRate.Denominator;
		std::wstring text =
			L"Width = " + std::to_wstring(x.Width) + L" " +
			L"Height = " + std::to_wstring(x.Height) + L" " +
			L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
			L"\n";

		::OutputDebugString(text.c_str());
	}
}