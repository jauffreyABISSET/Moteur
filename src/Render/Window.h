#pragma once

#include <Windows.h>
#include <string>

class Window
{
public:
    Window() = default;
    explicit Window(HINSTANCE hInst, const std::wstring& caption, int width, int height)
        : mhAppInst(hInst), mMainWndCaption(caption), mClientWidth(width), mClientHeight(height) {
    }

    bool Init(HINSTANCE hInst, const std::wstring& caption, int width, int height);

    HINSTANCE AppInst() const;
    HWND MainWnd() const;
    float AspectRatio() const;

    void SetTitle(const std::wstring& caption);
    void SetClientWidth(int width);
    void SetClientHeight(int height);
    void SetClientSize(int width, int height);

    const std::wstring& GetCaption() const;
    int GetClientWidth() const;
    int GetClientHeight() const;
    HWND GetHandle() const;
    std::wstring GetTitle() const;

private:
    HINSTANCE   mhAppInst = nullptr;
    HWND        mhMainWnd = nullptr; 
    std::wstring mMainWndCaption = L"d3d App";
    int         mClientWidth = 800;
    int         mClientHeight = 600;
};