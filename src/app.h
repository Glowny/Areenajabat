#pragma once
namespace arena
{
    struct Event;

    struct App
    {
        // return true if exit
        bool update();

        void init();

        void shutdown();
    };

    const Event* poll();
    void release(const Event* event);
}