#pragma once

#include "..\types.h"

const uint32 Mem1K		= 1024;
const uint32 Mem2K		= Mem1K * 2;
const uint32 Mem4K		= Mem2K * 2;
const uint32 Mem8K		= Mem4K * 2;
const uint32 Mem16K		= Mem8K * 2;
const uint32 Mem32K		= Mem16K * 2;
const uint32 Mem64K		= Mem32K * 2;
const uint32 Mem128K	= Mem64K * 2;
const uint32 Mem256K	= Mem128K * 2;
const uint32 Mem512K	= Mem256K * 2;
const uint32 Mem1024K	= Mem512K * 2;

#define DYNAMIC_NEW(ptr, type, ...)		new (ptr)type(##__VA_ARGS__)

#define DYNAMIC_NEW_DEFAULT(ptr, type)	new(ptr)type()

#define DYNAMIC_DTOR(ptr, type)			ptr->~type()