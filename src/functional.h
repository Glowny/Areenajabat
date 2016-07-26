#pragma once
#include <bx/macros.h>
BX_PRAGMA_DIAGNOSTIC_PUSH()
BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4265) //'std::_Func_base<_Ret>': class has virtual functions, but destructor is not virtual
#include <functional>

template<typename T>
using Predicate = std::function<bool(T)>;

template<typename T1>
using Action1 = std::function<void(T1)>;

template<typename T1, typename T2>
using Action2 = std::function<void(T1, T2)>;

template<typename T1, typename T2, typename T3>
using Action3 = std::function<void(T1, T2, T3)>;

template<typename T1, typename T2, typename T3, typename T4>
using Action4 = std::function<void(T1, T2, T3, T4)>;

template<typename T1, typename T2, typename T3, typename T4, typename T5>
using Action5 = std::function<void(T1, T2, T3, T4, T5)>;

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
using Action6 = std::function<void(T1, T2, T3, T4, T5, T6)>;

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
using Action7 = std::function<void(T1, T2, T3, T4, T5, T6, T7)>;

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
using Action8 = std::function<void(T1, T2, T3, T4, T5, T6, T7, T8)>;

BX_PRAGMA_DIAGNOSTIC_POP()