#pragma once

#define DYNAMIC_NEW(ptr, type, ...)		new (ptr)type(##__VA_ARGS__)

#define DYNAMIC_NEW_DEFAULT(ptr, type)	new(ptr)type()

#define DYNAMIC_DTOR(ptr, type)			ptr->~type()
