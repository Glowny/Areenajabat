#pragma once
namespace arena
{
    struct Event;

    struct App
    {
        void update();

        void init();

        void shutdown();
    };

    const Event* poll();
    void release(const Event* event);
}