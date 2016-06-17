#pragma once
#include <stdint.h>
#include <bx/allocator.h>

namespace arena
{
    struct Event;
    class ResourceManager;

    struct App
    {
        // return true if exit
        bool update();

        void init(int32_t width, int32_t height);

        void shutdown();

    private:
        int32_t width;
        int32_t height;

    };

    const Event* poll();
    void release(const Event* event);

    void setWindowSize(uint32_t width, uint32_t height, bool force = false);

    ResourceManager* getResources();

    bx::AllocatorI* getAllocator();
}