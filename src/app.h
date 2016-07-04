#pragma once

#include <stdint.h>
#include <bx/allocator.h>

#include "forward_declare.h"
#include "camera.h"

FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, SpriteBatch)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, ResourceManager)

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
		App();

        int32_t				m_width			{ 0 };
        int32_t				m_height		{ 0 };
		ResourceManager*	m_resources		{ nullptr };
		SpriteBatch*		m_spriteBatch	{ nullptr };
		Camera				m_camera;
    };

    struct Mouse
    {
        static const struct MouseState& getState();
    };

    const Event* poll();

    void release(const Event* event);

    void setWindowSize(uint32_t width, uint32_t height, bool force = false);

    bx::AllocatorI* getAllocator();
}