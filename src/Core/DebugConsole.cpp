#include "pch.h"
#include "DebugConsole.h"

#include <fcntl.h>
#include <io.h>
#include <iostream>

void CreateDebugConsole(const char* title)
{
    if (!AllocConsole())
    {
        // Si échec, on peut interroger GetLastError() depuis le débogueur
        return;
    }

    // Redirige les flux C vers la console
    FILE* fp = nullptr;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    freopen_s(&fp, "CONIN$", "r", stdin);

    // Désactiver le buffering pour éviter d'attendre flush explicite
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
    setvbuf(stdin, nullptr, _IONBF, 0);

    // Synchroniser et nettoyer les iostream C++
    std::ios::sync_with_stdio(true);
    std::cout.clear();
    std::cerr.clear();
    std::cin.clear();
    std::wcout.clear();
    std::wcin.clear();
    std::wcerr.clear();

    // UTF-8 et titre utile
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleTitleA(title);
}

void DestroyDebugConsole()
{
    std::fflush(stdout);
    std::fflush(stderr);

    // Fermer les FILE* réassignés
    if (stdout) std::fclose(stdout);
    if (stderr) std::fclose(stderr);
    if (stdin)  std::fclose(stdin);

    FreeConsole();
}