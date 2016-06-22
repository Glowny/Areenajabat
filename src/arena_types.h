#pragma once

#include "inttypes.h"
#include <string>

// Guess there is no need for any assertions as we 
// are only targeting Linux and Windows platforms.

/*
	Pointer types.
*/

using IntPtr	= intptr_t;
using VoidPtr	= void*;

/*
	Char/string types.
*/

using Char		= char;
using CharArray	= char*;
using String	= std::string;

/*
	Signed types.
*/

using int8		= int8_t;
using int16		= int16_t;
using int32		= int32_t;
using int64		= int64_t;

/*
	Unsigned types.
*/

using uint8		= uint8_t;
using uint16	= uint16_t;
using uint32	= uint32_t;
using uint64	= uint64_t;

/*
	Floating point types.
*/

using float32	= float;
using float64	= double;