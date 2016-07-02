#pragma once
#include <stdint.h>
#include <bx/allocator.h>
#include "graphics\spritebatch.h"

namespace arena
{
    struct Event;
    class ResourceManager;

    struct App
    {
		static App& instance();

        // return true if exit
        bool update();

        void init(int32_t width, int32_t height);

        void shutdown();

		SpriteBatch* const spriteBatch();
		ResourceManager* const resources();
		Camera& camera();
		
		~App() = default;

		App(App const& copy) = delete;
		App& operator=(App const& copy) = delete;
	private:
		App() = default;

        int32_t				m_width;
        int32_t				m_height;
		ResourceManager*	m_resources;
		SpriteBatch*		m_spriteBatch;
		Camera				m_camera;
    };

    const Event* poll();

    void release(const Event* event);

    void setWindowSize(uint32_t width, uint32_t height, bool force = false);

    ResourceManager* getResources();

    bx::AllocatorI* getAllocator();
}