#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <stop_token>

class Thread
{
public:
    Thread();
    virtual ~Thread();

    // Initialise le thread (stocke un nom, optionnel)
    void CreateThread(const std::string& name);

    // Démarre l'exécution (lance std::jthread qui appellera Run)
    void Start();

    // Demande l'arrêt et attend la fin
    void Stop();

    bool IsRunning() const;

protected:
    // Méthode à surcharger par les classes filles.
    // Le stop token permet un arrêt coopératif.
    virtual void Run(std::stop_token st);

private:
    std::jthread m_thread;
    std::atomic_bool m_running{ false };
    std::string m_name;
};