#include "bgfx_utils.h"
#include <bx/readerwriter.h>
#include "../app.h"

#define LODEPNG_NO_COMPILE_ENCODER
#define LODEPNG_NO_COMPILE_DISK
#define LODEPNG_NO_COMPILE_ANCILLARY_CHUNKS
#define LODEPNG_NO_COMPILE_ERROR_TEXT
#define LODEPNG_NO_COMPILE_ALLOCATORS
#define LODEPNG_NO_COMPILE_CPP
#include <lodepng/lodepng.cpp>

void* lodepng_malloc(size_t _size)
{
    return ::malloc(_size);
}

void* lodepng_realloc(void* _ptr, size_t _size)
{
    return ::realloc(_ptr, _size);
}

void lodepng_free(void* _ptr)
{
    ::free(_ptr);
}

#include <bx/endian.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.c>

#include <assert.h>
#include "../dbg.h"

namespace arena
{
    namespace utils
    {
        static const bgfx::Memory* loadMem(bx::FileReaderI* reader, const char* path)
        {
            if (bx::open(reader, path))
            {
                uint32_t size = (uint32_t)bx::getSize(reader);
                const bgfx::Memory* mem = bgfx::alloc(size + 1);
                bx::read(reader, mem->data, size);
                bx::close(reader);
                mem->data[mem->size - 1] = '\0';

                return mem;
            }
            return NULL;
        }

        static void* loadMem(bx::FileReaderI* _reader, bx::AllocatorI* _allocator, const char* _filePath, uint32_t* _size)
        {
            if (bx::open(_reader, _filePath))
            {
                uint32_t size = (uint32_t)bx::getSize(_reader);
                void* data = BX_ALLOC(_allocator, size);
                bx::read(_reader, data, size);
                bx::close(_reader);

                if (NULL != _size)
                {
                    *_size = size;
                }
                return data;
            }

            return NULL;
        }

        static bgfx::ShaderHandle loadShader(bx::FileReaderI* reader, const char* name)
        {
            char filePath[512] = { 0 };
            const char* shaderPath =
                bgfx::getRendererType() == bgfx::RendererType::Direct3D11 ?
                "assets/shaders/dx11/" : "assets/shaders/gl/";

            strcpy(filePath, shaderPath);
            strcat(filePath, name);
            strcat(filePath, ".bin");

            return bgfx::createShader(loadMem(reader, filePath));
        }

        bgfx::ProgramHandle loadProgram(bx::FileReaderI* reader, const char* vshName, const char* fshName)
        {
            bgfx::ShaderHandle vsh = loadShader(reader, vshName);
            bgfx::ShaderHandle fsh = loadShader(reader, fshName);

            return bgfx::createProgram(vsh, fsh, true);
        }

        bgfx::TextureHandle loadTexture(bx::FileReaderI* reader, const char* filePath, uint32_t flags, bgfx::TextureInfo* info)
        {
            bgfx::TextureHandle handle = BGFX_INVALID_HANDLE;
            bx::AllocatorI* allocator = arena::getAllocator();

            uint32_t size = 0;
            void* data = loadMem(reader, allocator, filePath, &size);

            if (data != NULL)
            {
                bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8;
                uint32_t bpp = 32;

                uint32_t width = 0;
                uint32_t height = 0;


                typedef void(*ReleaseFn)(void* _ptr);
                ReleaseFn release = stbi_image_free;

                uint8_t* out = NULL;
                static uint8_t pngMagic[] = { 0x89, 0x50, 0x4E, 0x47, 0x0d, 0x0a };
                if (0 == memcmp(data, pngMagic, sizeof(pngMagic)))
                {
                    release = lodepng_free;

                    unsigned error;
                    LodePNGState state;
                    lodepng_state_init(&state);
                    state.decoder.color_convert = 0;
                    error = lodepng_decode(&out, &width, &height, &state, (uint8_t*)data, size);

                    if (0 == error)
                    {
                        switch (state.info_raw.bitdepth)
                        {
                        case 8:
                            switch (state.info_raw.colortype)
                            {
                            case LCT_GREY:
                                format = bgfx::TextureFormat::R8;
                                bpp = 8;
                                break;

                            case LCT_GREY_ALPHA:
                                format = bgfx::TextureFormat::RG8;
                                bpp = 16;
                                break;

                            case LCT_RGB:
                                format = bgfx::TextureFormat::RGB8;
                                bpp = 24;
                                break;

                            case LCT_RGBA:
                                format = bgfx::TextureFormat::RGBA8;
                                bpp = 32;
                                break;

                            case LCT_PALETTE:
                                break;
                            }
                            break;

                        case 16:
                            switch (state.info_raw.colortype)
                            {
                            case LCT_GREY:
                                for (uint32_t ii = 0, num = width*height; ii < num; ++ii)
                                {
                                    uint16_t* rgba = (uint16_t*)out + ii * 4;
                                    rgba[0] = bx::toHostEndian(rgba[0], false);
                                }
                                format = bgfx::TextureFormat::R16;
                                bpp = 16;
                                break;

                            case LCT_GREY_ALPHA:
                                for (uint32_t ii = 0, num = width*height; ii < num; ++ii)
                                {
                                    uint16_t* rgba = (uint16_t*)out + ii * 4;
                                    rgba[0] = bx::toHostEndian(rgba[0], false);
                                    rgba[1] = bx::toHostEndian(rgba[1], false);
                                }
                                format = bgfx::TextureFormat::R16;
                                bpp = 16;
                                break;

                            case LCT_RGBA:
                                for (uint32_t ii = 0, num = width*height; ii < num; ++ii)
                                {
                                    uint16_t* rgba = (uint16_t*)out + ii * 4;
                                    rgba[0] = bx::toHostEndian(rgba[0], false);
                                    rgba[1] = bx::toHostEndian(rgba[1], false);
                                    rgba[2] = bx::toHostEndian(rgba[2], false);
                                    rgba[3] = bx::toHostEndian(rgba[3], false);
                                }
                                format = bgfx::TextureFormat::RGBA16;
                                bpp = 64;
                                break;

                            case LCT_RGB:
                            case LCT_PALETTE:
                                break;
                            }
                            break;

                        default:
                            break;
                        }
                    }

                    lodepng_state_cleanup(&state);
                }
                else
                {
                    int comp = 0;
                    out = stbi_load_from_memory((uint8_t*)data, size, (int*)&width, (int*)&height, &comp, 4);
                }

                BX_FREE(allocator, data);

                if (NULL != out)
                {
                    handle = bgfx::createTexture2D(uint16_t(width), uint16_t(height), 1
                        , format
                        , flags
                        , bgfx::copy(out, width*height*bpp / 8)
                    );
                    release(out);

                    if (NULL != info)
                    {
                        bgfx::calcTextureSize(*info
                            , uint16_t(width)
                            , uint16_t(height)
                            , 0
                            , false
                            , 1
                            , format
                        );
                    }
                }
                else
                {
                    DBG("failed to load %s", filePath);
                }
            }

            return handle;
        }
    }
}