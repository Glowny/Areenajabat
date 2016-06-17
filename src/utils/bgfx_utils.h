#pragma once

#include <bgfx/bgfx.h>
#include <bx/readerwriter.h>

namespace arena
{
    namespace utils
    {
        bgfx::ShaderHandle loadShader(bx::FileReaderI* reader, const char* name);

        bgfx::ProgramHandle loadProgram(bx::FileReaderI* reader, const char* vshName, const char* fshName);

        bgfx::TextureHandle loadTexture(bx::FileReaderI* reader, const char* filePath, uint32_t flags, bgfx::TextureInfo* info);
    }
}