#include "spriter_resource.h"
#include <bx/macros.h>

BX_PRAGMA_DIAGNOSTIC_PUSH_MSVC()
BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4263) // 'function' : member function does not override any base class virtual member function
#include <spriterengine/spriterengine.h>
BX_PRAGMA_DIAGNOSTIC_POP_MSVC()

namespace arena
{
    namespace spriter
    {
        void* load(const std::string name)
        {
            return NULL;
        }

        void unload(void*)
        {

        }
    }
}

