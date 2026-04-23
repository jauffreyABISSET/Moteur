#pragma once
#include "Core/Thread.h"

class Worker : public Thread {

protected:

    void Run(std::stop_token st) override {
        RenderSystem& render = RenderSystem::Get();
        while (!st.stop_requested()) {


            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        std::cout << "stopping\n";
    }
};