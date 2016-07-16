#pragma once

#include <stdint.h>

namespace arena
{
#if _DEBUG
#	define DEBUG_PRINT(__str__) fprintf(stderr, __str__)
#elif
#	define DEBUG_PRINT(__str__) 
#endif

    void trace(const char* path, uint16_t line, const char* format, ...);
    void fatal(const char* path, uint16_t line, const char* format, ...);
}


#ifdef _DEBUG
#   define ARENA_TRACE(_format, ...)                                                                                                 \
    do                                                                                                                               \
    {                                                                                                                                \
        arena::trace(__FILE__, uint16_t(__LINE__), _format "\n", ##__VA_ARGS__);                                                     \
    } while(0)


#   define ARENA_ASSERT(_condition, _msg, ...)                                                                                        \
      do                                                                                                                             \
      {                                                                                                                              \
          if (!(_condition))                                                                                                         \
          {                                                                                                                          \
                ARENA_TRACE("\nAssertion failed: %s with message " _msg "\n", #_condition, ##__VA_ARGS__);                           \
                arena::fatal(__FILE__, uint16_t(__LINE__), "\nAssertion failed: %s\n\t " _msg "\n", #_condition, ##__VA_ARGS__);     \
          }                                                                                                                          \
       } while(0)


#else
#   define ARENA_TRACE(_format, ...) do {} while (0)
#   define ARENA_ASSERT(_condition, _format, ...) do {} while (0)
#endif