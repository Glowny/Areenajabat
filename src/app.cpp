#include "app.h"
#include <bgfx/bgfx.h>
#include "input/input.h"
#include "input/event.h"
#include <bx/bx.h>
#include <bx/crtimpl.h>
#include "res/resource_manager.h"
#include <bx/fpumath.h>
#include "io/io.h"
#include "utils/bgfx_utils.h"
#include <glm/gtx/matrix_transform_2d.hpp>

namespace arena
{
    static bool s_exit = false;
    static uint32_t s_reset = BGFX_RESET_NONE;

    static MouseState s_mouseState;

    static ResourceManager* s_resources;

    static void cmdExit(const void*)
    {
        s_exit = true;
    }

    static const InputBinding s_bindings[] =
    {
        { arena::Key::KeyQ, arena::Modifier::LeftCtrl, 0, cmdExit, "exit" },
        INPUT_BINDING_END
    };

    struct PosUvColorVertex
    {
        float m_x;
        float m_y;
        float m_u;
        float m_v;
        uint32_t m_abgr;

        static void init()
        {
            ms_decl
                .begin()
                .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
                .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
                .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
                .end();
        }

        static bgfx::VertexDecl ms_decl;
    };

    bgfx::VertexDecl PosUvColorVertex::ms_decl;

    bool screenQuad(int32_t _x, int32_t _y, int32_t _width, uint32_t _height, uint32_t _abgr, bool _originBottomLeft = false)
    {
        if (bgfx::checkAvailTransientVertexBuffer(6, PosUvColorVertex::ms_decl))
        {
            bgfx::TransientVertexBuffer vb;
            bgfx::allocTransientVertexBuffer(&vb, 6, PosUvColorVertex::ms_decl);
            PosUvColorVertex* vertex = (PosUvColorVertex*)vb.data;

            const float widthf = float(_width);
            const float heightf = float(_height);

            const float minx = float(_x);
            const float miny = float(_y);
            const float maxx = minx + widthf;
            const float maxy = miny + heightf;

            float m_halfTexel = 0.0f;

            const float texelHalfW = m_halfTexel / widthf;
            const float texelHalfH = m_halfTexel / heightf;
            const float minu = texelHalfW;
            const float maxu = 1.0f - texelHalfW;
            const float minv = _originBottomLeft ? texelHalfH + 1.0f : texelHalfH;
            const float maxv = _originBottomLeft ? texelHalfH : texelHalfH + 1.0f;

            vertex[0].m_x = minx;
            vertex[0].m_y = miny;
            vertex[0].m_u = minu;
            vertex[0].m_v = minv;

            vertex[1].m_x = maxx;
            vertex[1].m_y = miny;
            vertex[1].m_u = maxu;
            vertex[1].m_v = minv;

            vertex[2].m_x = maxx;
            vertex[2].m_y = maxy;
            vertex[2].m_u = maxu;
            vertex[2].m_v = maxv;

            vertex[3].m_x = maxx;
            vertex[3].m_y = maxy;
            vertex[3].m_u = maxu;
            vertex[3].m_v = maxv;

            vertex[4].m_x = minx;
            vertex[4].m_y = maxy;
            vertex[4].m_u = minu;
            vertex[4].m_v = maxv;

            vertex[5].m_x = minx;
            vertex[5].m_y = miny;
            vertex[5].m_u = minu;
            vertex[5].m_v = minv;

            vertex[0].m_abgr = _abgr;
            vertex[1].m_abgr = _abgr;
            vertex[2].m_abgr = _abgr;
            vertex[3].m_abgr = _abgr;
            vertex[4].m_abgr = _abgr;
            vertex[5].m_abgr = _abgr;

            bgfx::setVertexBuffer(&vb);

            return true;
        }

        return false;
    }

    bgfx::IndexBufferHandle ibh;
    bgfx::VertexBufferHandle vbh;
    bgfx::UniformHandle s_texture;
    bgfx::TextureHandle texture;
    bgfx::ProgramHandle program;
    struct Sprite
    {
        Sprite()
            : m_origin(0, 0),
            m_transform(1.f),
            m_position(0.f),
            m_angle(2 * 3.14f)
        {

        }



        void draw()
        {
            m_origin = glm::vec2(m_res->width / 2.f, m_res->height / 2.f);
            m_transform =
                //glm::translate(glm::mat3(1.f), m_position) *
                glm::rotate(glm::mat3(1.f), m_angle);
            glm::vec3 points[4] =
            {
                m_transform * glm::vec3(m_position, 1.f),
                m_transform * glm::vec3(m_position.x + m_res->width, m_position.y, 1.f),
                m_transform * glm::vec3(m_position.x + m_res->width, m_position.y + m_res->height, 1.f),
                m_transform * glm::vec3(m_position.x, m_position.y + m_res->height, 1.f),
            };

            bool _originBottomLeft = bgfx::getRendererType() == bgfx::RendererType::OpenGL ? true : false;
            uint32_t _abgr = 0xFFFFFFFF;
            if (bgfx::checkAvailTransientVertexBuffer(6, PosUvColorVertex::ms_decl))
            {
                bgfx::TransientVertexBuffer vb;
                bgfx::allocTransientVertexBuffer(&vb, 6, PosUvColorVertex::ms_decl);
                PosUvColorVertex* vertex = (PosUvColorVertex*)vb.data;

                const float widthf = float(m_res->width);
                const float heightf = float(m_res->height);

                float m_halfTexel = 0.0f;

                const float texelHalfW = m_halfTexel / widthf;
                const float texelHalfH = m_halfTexel / heightf;
                const float minu = texelHalfW;
                const float maxu = 1.0f - texelHalfW;
                const float minv = _originBottomLeft ? texelHalfH + 1.0f : texelHalfH;
                const float maxv = _originBottomLeft ? texelHalfH : texelHalfH + 1.0f;

                vertex[0].m_x = points[0].x; // left
                vertex[0].m_y = points[0].y;
                vertex[0].m_u = minu;
                vertex[0].m_v = minv;

                vertex[1].m_x = points[1].x;        // right
                vertex[1].m_y = points[1].y;
                vertex[1].m_u = maxu;
                vertex[1].m_v = minv;

                vertex[2].m_x = points[2].x;  
                vertex[2].m_y = points[2].y;
                vertex[2].m_u = maxu;
                vertex[2].m_v = maxv;

                vertex[3].m_x = points[2].x;
                vertex[3].m_y = points[2].y;
                vertex[3].m_u = maxu;
                vertex[3].m_v = maxv;

                vertex[4].m_x = points[3].x;
                vertex[4].m_y = points[3].y;
                vertex[4].m_u = minu;
                vertex[4].m_v = maxv;

                vertex[5].m_x = points[0].x;
                vertex[5].m_y = points[0].y;
                vertex[5].m_u = minu;
                vertex[5].m_v = minv;

                vertex[0].m_abgr = _abgr;
                vertex[1].m_abgr = _abgr;
                vertex[2].m_abgr = _abgr;
                vertex[3].m_abgr = _abgr;
                vertex[4].m_abgr = _abgr;
                vertex[5].m_abgr = _abgr;

                bgfx::setVertexBuffer(&vb);
            }

        }
        glm::mat3 m_transform;
        glm::vec2 m_position;
        glm::vec2 m_origin;
        float m_angle;
        TextureResource* m_res;
    };

    static Sprite s_sprite;

    void App::init(int32_t width, int32_t height)
    {
        this->width = width;
        this->height = height;

        char workingdir[512];


#if _WIN32 || _WIN64
        // TODODODODODO change dir to folder root
        bx::chdir("..\\..\\..\\");
        bx::pwd(workingdir, 512);
        printf("CWD: %s\n", workingdir);
#endif
        inputInit();
        inputAddBindings("bindings", s_bindings);

        bgfx::reset(width, height, s_reset);
        bgfx::setDebug(BGFX_DEBUG_TEXT);

        // Set view 0 clear state.
        bgfx::setViewClear(0
            , BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
            , 0x303030ff
            , 1.0f
            , 0
            );

        PosUvColorVertex::init();

        s_resources = new ResourceManager("assets/");

        s_texture = bgfx::createUniform("s_texColor", bgfx::UniformType::Int1);
        program = getResources()->get<ProgramResource>(ResourceType::Shader, "basic")->handle;
        s_sprite.m_res = getResources()->get<TextureResource>(ResourceType::Texture, "perkele.png");
        s_sprite.m_position = glm::vec2(100, 100);
        texture = s_sprite.m_res->handle;
        

        
    }



    

    bool App::update()
    {
        const Event* ev;

        uint32_t reset = s_reset;

        do {
            struct SE { const Event* m_ev; SE() : m_ev(poll()) {} ~SE() { if (NULL != m_ev) { release(m_ev); } } } scopeEvent;
            ev = scopeEvent.m_ev;

            SDL_Window* window = nullptr;

            if (NULL != ev)
            {
                switch (ev->m_type)
                {
                    case Event::Char:
                    {
                        const CharEvent* chev = static_cast<const CharEvent*>(ev);
                        inputChar(chev->m_len, chev->m_char);
                    }
                    break;
                    case Event::Exit:
                        return true;
                    case Event::Mouse:
                    {
                        const MouseEvent* mouse = static_cast<const MouseEvent*>(ev);
                        window = mouse->m_window;

                        if (mouse->m_move)
                        {
                            inputSetMousePos(mouse->m_mx, mouse->m_my, mouse->m_mz);
                            s_mouseState.m_mx = mouse->m_mx;
                            s_mouseState.m_my = mouse->m_my;
                            s_mouseState.m_mz = mouse->m_mz;
                        }
                        else
                        {
                            inputSetMouseButtonState(mouse->m_button, mouse->m_down);
                            s_mouseState.m_buttons[mouse->m_button] = mouse->m_down;
                        }
                    }
                    break;
                    case Event::Key:
                    {
                        const KeyEvent* key = static_cast<const KeyEvent*>(ev);
                        window = key->m_window;

                        inputSetKeyState(key->m_key, key->m_modifiers, key->m_down);
                    }
                    break;

                    case Event::Size:
                    {
                        const SizeEvent* size = static_cast<const SizeEvent*>(ev);
                        window = size->m_window;
                        width = size->m_width;
                        height = size->m_height;
                        reset = !s_reset; // force reset
                    }
                    break;
                    default:
                        break;
                }
            }

            inputProcess();

        } while (ev != NULL);

        if (reset != s_reset)
        {
            bgfx::reset(width, height, reset);
            inputSetMouseResolution(uint16_t(width), uint16_t(height));
        }

        float ortho[16];
        bx::mtxOrtho(ortho, 0.0f, float(width), float(height), 0.0f, 0.0f, 1000.0f);
        bgfx::setViewTransform(0, NULL, ortho);
        bgfx::setViewRect(0, 0, 0, uint16_t(width), uint16_t(height));

        bgfx::touch(0);
        bgfx::dbgTextClear();
        bgfx::dbgTextPrintf(0, 1, 0x4f, "Perkeleen perkele");
        bgfx::dbgTextPrintf(0, 2, 0x6f, "Mouse x = %d, y = %d, wheel = %d", s_mouseState.m_mx, s_mouseState.m_my, s_mouseState.m_mz);
        bgfx::dbgTextPrintf(0, 3, 0x8f, "Left btn = %s, Middle btn = %s, Right btn = %s", 
            s_mouseState.m_buttons[MouseButton::Left] ? "down" : "up", 
            s_mouseState.m_buttons[MouseButton::Middle] ? "down" : "up", 
            s_mouseState.m_buttons[MouseButton::Right] ? "down" : "up");



        bgfx::setTexture(0, s_texture, texture);

        /*
        screenQuad(256, 256, 256, 256, 0xFFFFFF00, false);*/

        s_sprite.m_position = glm::vec2(100.f, 0.f);
        s_sprite.draw();

        // Set render states.
        bgfx::setState(0
            | BGFX_STATE_RGB_WRITE
            | BGFX_STATE_ALPHA_WRITE
            | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
            );
        bgfx::submit(0, program);

        bgfx::frame();

        return s_exit;
    }

    void App::shutdown()
    {
        inputRemoveBindings("bindings");
        inputShutdown();
    }

    bx::AllocatorI* getAllocator()
    {
        static bx::CrtAllocator s_allocator;
        return &s_allocator;
    }

    ResourceManager* getResources()
    {
        return s_resources;
    }
}