#include "pch.h"
#include "GameManager.h"

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ PSTR cmdLine,
    _In_ int cmdShow
)
{
    (void)hPrevInstance;
    (void)cmdLine;
    (void)cmdShow;

    GameManager* pGameManager = GameManager::GetInstance();

    pGameManager->Init(hInstance);
    pGameManager->Run();
    pGameManager->Destroy();

    return 0;
}