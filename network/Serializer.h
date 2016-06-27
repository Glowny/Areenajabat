#pragma once
#include "Enumerations.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>


template <class... T>
struct Unpack {};

template<>
struct Unpack<> {
	static void apply() {}
};

template <class Head, class... Tail>
struct Unpack<Head, Tail...> {
	static void apply(Head value) {
		+= sizeof(Head)
	}
};

template <typename... Args>
void serialize(Args... args)
{
	Unpack<Args...>::apply(args);
}


// raw version, could be more automatic
unsigned char* serialize(DataType dataTypes[], size_t argumentAmount, ...)
{
	serialize<int, double, float>();
	va_list listPointer;
	va_start(listPointer, argumentAmount);
	
	unsigned char* data;
	size_t index = 0;
	
	DataType dataType;
	
	for (unsigned i = 0; i < argumentAmount; i++)
	{
		dataType = dataTypes[i];
		switch (dataType)
		{
		case unsignedInt:
			*((unsigned*)(&data[index])) = va_arg(listPointer, unsigned);
			index += sizeof(unsigned);
			break;
		case Int:
			*((int*)(&data[index])) = va_arg(listPointer, int);
			index += sizeof(int);
			break;
		case Float:
			*((float*)(&data[index])) = va_arg(listPointer, float);
			index += sizeof(float);
			break;
		case Char:
			*((char*)(&data[index])) = va_arg(listPointer, char);
			index += sizeof(char);
			break;
		default:
			printf("INVALID ARGUMENT on serialization!\n");
			exit(1);
			break;
		}
	}
	va_end(listPointer);
	return data;
}
void deSerialize(DataType dataTypes[], size_t argumentAmount, unsigned char* data, ...)
{
	va_list listPointer;
	va_start(listPointer, argumentAmount);

	size_t index = 0;

	DataType dataType;

	for (unsigned i = 0; i < argumentAmount; i++)
	{
		dataType = dataTypes[i];
		switch (dataType)
		{
		case unsignedInt:
			unsigned* arg = va_arg(listPointer, unsigned*);
			*arg  = *((unsigned*)(&data[index]));
			index += sizeof(unsigned);
			break;
		case Int:
			int* arg = va_arg(listPointer, int*);
			*arg = *((int*)(&data[index]));
			index += sizeof(int);
			break;
		case Float:
			float* arg = va_arg(listPointer, float*);
			*arg = *((float*)(&data[index]));
			index += sizeof(float);
			break;
		case Char:
			char* arg = va_arg(listPointer, char*);
			*arg = *((char*)(&data[index]));
			index += sizeof(char);
			break;
		default:
			printf("INVALID ARGUMENT on deSerialization!\n");
			exit(1);
			break;
		}
	}
	va_end(listPointer);

}
unsigned getID(unsigned char* data)
{
	return *((unsigned*)(&data[0]));
	
	return 0;
}

unsigned char* reserveSpace(DataType dataTypes[], size_t argumentAmount)
{
	unsigned char data;
	size_t size = 0;
	DataType dataType;
	for (unsigned i = 0; i < argumentAmount; i++)
	{
		dataType = dataTypes[i];
		switch (dataType)
		{
		case unsignedInt:
			size += sizeof(unsigned);
			break;
		case Int:
			size += sizeof(int);
			break;
		case Float:
			size += sizeof(float);
			break;
		case Char:
			size += sizeof(char);
			break;
		default:
			printf("INVALID ARGUMENT on deSerialization!\n");
			exit(1);
			break;
		}

	}
	data = (unsigned char)malloc(size);
}