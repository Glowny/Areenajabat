#pragma once

#include <bgfx/bgfx.h>
#include <bx/readerwriter.h>

namespace arena
{
    namespace utils
    {
        bgfx::ShaderHandle loadShader(bx::FileReaderI* reader, const char* name);

        bgfx::ProgramHandle loadProgram(bx::FileReaderI* reader, const char* vshName, const char* fshName);
    }
}