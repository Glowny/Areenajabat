#include <bgfx/bgfx.h>
#include <string>

#define DEFAULT_WIDTH 1280
#define DEFAULT_HEIGHT 720

#if _WIN32 || _WIN64
#	define SDL_MAIN_HANDLED
#endif

#include <bx/bx.h>
#include <bx/thread.h>
#include <SDL2/SDL.h>

BX_PRAGMA_DIAGNOSTIC_PUSH_CLANG()
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG("-Wextern-c-compat")
#include <SDL2/SDL_syswm.h>
BX_PRAGMA_DIAGNOSTIC_POP_CLANG()

#include <bgfx/bgfxplatform.h>
#if defined(None) // X11 defines this...
#	undef None
#endif // defined(None)

static bool s_exit = false;

int32_t thread_proc(void*);

struct Context
{
    int run()
    {
        int32_t width = 1280;
        int32_t height = 720;

        SDL_Init(SDL_INIT_GAMECONTROLLER);

        SDL_Window* wnd = SDL_CreateWindow("bgfx"
            , SDL_WINDOWPOS_UNDEFINED
            , SDL_WINDOWPOS_UNDEFINED
            , width
            , height
            , SDL_WINDOW_SHOWN
            | SDL_WINDOW_RESIZABLE
        );

        bgfx::sdlSetWindow(wnd);
        bgfx::renderFrame();

        m_thread.init(thread_proc);

        bool exit = false;

        SDL_Event event;
        while (!exit)
        {
            bgfx::renderFrame();

            while (SDL_PollEvent(&event))
                switch (event.type)
                {
                case SDL_QUIT:
                    //m_eventQueue.postExitEvent();
                    exit = true;
                    s_exit = true;
                    break;
                }
        }

        while (bgfx::RenderFrame::NoContext != bgfx::renderFrame()) {};

        m_thread.shutdown();

        SDL_DestroyWindow(wnd);
        SDL_Quit();

        return m_thread.getExitCode();
        //bgfx::shutdown();
    }

    bx::Thread m_thread;
};

static Context s_ctx;

int32_t thread_proc(void*)
{
    bgfx::init();
    bgfx::reset(1280, 720, BGFX_DEBUG_TEXT);

    // Enable debug text.
    bgfx::setDebug(BGFX_DEBUG_TEXT);

    // Set view 0 clear state.
    bgfx::setViewClear(0
        , BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
        , 0x303030ff
        , 1.0f
        , 0
    );

    while (!s_exit) 
    {
        // Set view 0 default viewport.
        bgfx::setViewRect(0, 0, 0, uint16_t(1280), uint16_t(720));

        // This dummy draw call is here to make sure that view 0 is cleared
        // if no other draw calls are submitted to view 0.
        bgfx::touch(0);

        bgfx::frame();
    }

    SDL_Event event;
    SDL_QuitEvent& qev = event.quit;
    qev.type = SDL_QUIT;
    SDL_PushEvent(&event);

    bgfx::shutdown();

    return 0;
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;
    s_ctx.run();

    return 0;
}