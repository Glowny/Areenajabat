#include <bgfx/bgfx.h>
#include <string>

#define DEFAULT_WIDTH 1280
#define DEFAULT_HEIGHT 720

#if _WIN32 || _WIN64
#	define SDL_MAIN_HANDLED
#endif

#include <bx/bx.h>
#include <SDL2/SDL.h>

BX_PRAGMA_DIAGNOSTIC_PUSH_CLANG()
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG("-Wextern-c-compat")
#include <SDL2/SDL_syswm.h>
BX_PRAGMA_DIAGNOSTIC_POP_CLANG()

#include <bgfx/bgfxplatform.h>
#if defined(None) // X11 defines this...
#	undef None
#endif // defined(None)



int main()
{
    int32_t width = 1280;
    int32_t height = 720;

    SDL_Init(SDL_INIT_GAMECONTROLLER);

    SDL_CreateWindow("bgfx"
        , SDL_WINDOWPOS_UNDEFINED
        , SDL_WINDOWPOS_UNDEFINED
        , width
        , height
        , SDL_WINDOW_SHOWN
        | SDL_WINDOW_RESIZABLE
    );

    bgfx::init(bgfx::RendererType::OpenGL, BGFX_PCI_ID_NVIDIA);
    bgfx::reset(1280, 720, BGFX_RESET_VSYNC);
    
    bgfx::setViewClear(0
        , BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
        , 0x303030ff
        , 1.0f
        , 0
    );

    bgfx::shutdown();

    return 0;
}