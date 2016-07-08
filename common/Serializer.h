#pragma once
#include "Enumerations.h"
#include <stdint.h>

template <typename T>
void serializeSingle(unsigned char*& dataPointer, T single)
{
	memcpy(dataPointer, &single, sizeof(T));
	dataPointer += sizeof(T);
}

template <typename T>
void deSerializeSingle(unsigned char*& dataPointer, T &single)
{
	memcpy(&single, dataPointer, sizeof(T));
	dataPointer += sizeof(T);
}

inline MessageIdentifier getID(unsigned char* data)
{
	return *((MessageIdentifier*)(&data[0]));
}



