#include "pch.h"
#include "Thread.h"
#include <chrono>
#include <iostream>

Thread::Thread() = default;

Thread::~Thread()
{
    Stop();
}

void Thread::CreateThread(const std::string& name)
{
    m_name = name;
}

void Thread::Start()
{
    if (m_running.load(std::memory_order_acquire))
        return;

    // lance un jthread qui appelle la méthode virtuelle Run
    m_running.store(true, std::memory_order_release);
    m_thread = std::jthread([this](std::stop_token st) {
        try {
            Run(st);
        }
        catch (...) {
            // loggez si nécessaire
        }
        m_running.store(false, std::memory_order_release);
    });
}

void Thread::Stop()
{
    if (m_thread.joinable())
    {
        m_thread.request_stop();
        // attendre la fin
        m_thread.join();
    }
    m_running.store(false, std::memory_order_release);
}

bool Thread::IsRunning() const
{
    return m_running.load(std::memory_order_acquire);
}

void Thread::Run(std::stop_token st)
{
    while (!st.stop_requested()) {
        // travail par défaut (ne rien faire)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}