#pragma once

#include <bgfx/bgfx.h>

namespace arena
{
    namespace utils
    {
        bgfx::ShaderHandle loadShader(const char* filePath);

        bgfx::ProgramHandle loadProgram(const char* vsFilePath, const char* fsFilePath);
    }
}