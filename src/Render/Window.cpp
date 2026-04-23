#include "pch.h"
#include "Window.h"


LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool Window::Init(HINSTANCE hInst, const std::wstring& caption, int width, int height)
{
	mhAppInst = hInst;

	// Create the main application window.
    WNDCLASS wc = {};
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = L"MainWnd";
	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", nullptr, 0);
		return false;
	}

	RECT R = { 0, 0, width, height };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int adjustedWidth = R.right - R.left;
	int adjustedHeight = R.bottom - R.top;
	HWND hwnd = CreateWindow(
		L"MainWnd", caption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		adjustedWidth, adjustedHeight, nullptr, nullptr, hInst, nullptr);
	if (!hwnd)
	{
		MessageBox(0, L"CreateWindow Failed.", nullptr, 0);
		return false;
	}
	mhMainWnd = hwnd;
	ShowWindow(hwnd, SW_SHOW);
	
	return true;
}

HINSTANCE Window::AppInst()const
{
	return mhAppInst;
}

HWND Window::MainWnd()const
{
	return mhMainWnd;
}

float Window::AspectRatio()const
{
	return static_cast<float>(mClientWidth) / mClientHeight;
}

const std::wstring& Window::GetCaption() const
{
	return mMainWndCaption;
}

void Window::SetTitle(const std::wstring& caption)
{
	mMainWndCaption = caption;
	SetWindowText(mhMainWnd, mMainWndCaption.c_str());
}

int Window::GetClientWidth() const
{
	return mClientWidth;
}

int Window::GetClientHeight() const
{
	return mClientHeight;
}

HWND Window::GetHandle() const
{
	return mhMainWnd;
}

std::wstring Window::GetTitle() const
{
	return mMainWndCaption;
}

void Window::SetClientWidth(int width)
{
	mClientWidth = width;
	RECT R = { 0, 0, width, mClientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int adjustedWidth = R.right - R.left;
	int adjustedHeight = R.bottom - R.top;
	SetWindowPos(mhMainWnd, nullptr, 0, 0, adjustedWidth, adjustedHeight, SWP_NOMOVE | SWP_NOZORDER);
}

void Window::SetClientHeight(int height)
{
	mClientHeight = height;
	RECT R = { 0, 0, mClientWidth, height };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int adjustedWidth = R.right - R.left;
	int adjustedHeight = R.bottom - R.top;
	SetWindowPos(mhMainWnd, nullptr, 0, 0, adjustedWidth, adjustedHeight, SWP_NOMOVE | SWP_NOZORDER);
}

void Window::SetClientSize(int width, int height)
{
	mClientWidth = width;
	mClientHeight = height;
	RECT R = { 0, 0, width, height };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int adjustedWidth = R.right - R.left;
	int adjustedHeight = R.bottom - R.top;
	SetWindowPos(mhMainWnd, nullptr, 0, 0, adjustedWidth, adjustedHeight, SWP_NOMOVE | SWP_NOZORDER);
}

