#pragma once
#include "Enumerations.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

// raw version, could be more automatic
inline unsigned char* serialize(unsigned char* data, DataType dataTypes[], size_t argumentAmount,  ...)
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
			*((unsigned*)(&data[index])) = va_arg(listPointer, unsigned);
			index += sizeof(unsigned);
			break;
		case Int:
			*((int*)(&data[index])) = va_arg(listPointer, int);
			index += sizeof(int);
			break;
		case Float:
			*((double*)(&data[index])) = va_arg(listPointer, double);
			index += sizeof(double);
			break;
		case Char:
			*((char*)(&data[index])) = va_arg(listPointer, char);
			index += sizeof(char);
			break;
		case messageID:
			*((MessageIdentifier*)(&data[index])) = va_arg(listPointer, MessageIdentifier);
			index += sizeof(MessageIdentifier);
			break;
		default:
			printf("INVALID ARGUMENT on serialization!\n");
			assert(0 && "Serialization failed");
			break;
		}
	}
	va_end(listPointer);
	return data;
}
inline unsigned char* serializeWithIndex(unsigned char *data, size_t &index, DataType dataTypes[],
	size_t argumentAmount, ...)
{

	va_list listPointer;
	va_start(listPointer, argumentAmount);

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
		{
			*((double*)(&data[index])) = va_arg(listPointer, double);
			index += sizeof(double);
			break;
		}
		case Char:
			*((char*)(&data[index])) = va_arg(listPointer, char);
			index += sizeof(char);
			break;
		case messageID:
			*((MessageIdentifier*)(&data[index])) = va_arg(listPointer, MessageIdentifier);
			index += sizeof(MessageIdentifier);
			break;
		default:
			printf("INVALID ARGUMENT on serialization!\n");
			assert(0 && "Serialization failed");
			break;
		}
	}
	va_end(listPointer);
	return data;
}
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


inline void deSerialize(unsigned char* data, DataType dataTypes[], size_t argumentAmount,  ...)
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
		{
			unsigned* arg = va_arg(listPointer, unsigned*);
			*arg = *((unsigned*)(&data[index]));
			index += sizeof(unsigned);
			break;
		}
		case Int:
		{
			int* arg = va_arg(listPointer, int*);
			*arg = *((int*)(&data[index]));
			index += sizeof(int);
			break;
		}
		case Float:
		{
			double* arg = va_arg(listPointer, double*);
			*arg = *((double*)(&data[index]));
			index += sizeof(double);
			break;
		}
		case Char:
		{
			char* arg = va_arg(listPointer, char*);
			*arg = *((char*)(&data[index]));
			index += sizeof(char);
			break;
		}
		case messageID:
		{
			MessageIdentifier* arg = va_arg(listPointer, MessageIdentifier*);
			*arg = *((MessageIdentifier*)(&data[index]));
			index += sizeof(MessageIdentifier);
			break;
		}
		default:
			printf("INVALID ARGUMENT on deSerialization!\n");
			assert(0 && "DeSerialization failed");
			break;
		}
	}
	va_end(listPointer);

}

inline void deSerializeWithIndex(unsigned char* data, size_t &index, DataType dataTypes[], size_t argumentAmount, ...)
{
	va_list listPointer;
	va_start(listPointer, argumentAmount);

	DataType dataType;

	for (unsigned i = 0; i < argumentAmount; i++)
	{
		dataType = dataTypes[i];
		switch (dataType)
		{
		case unsignedInt:
		{
			unsigned* arg = va_arg(listPointer, unsigned*);
			*arg = *((unsigned*)(&data[index]));
			index += sizeof(unsigned);
			break;
		}
		case Int:
		{
			int* arg = va_arg(listPointer, int*);
			*arg = *((int*)(&data[index]));
			index += sizeof(int);
			break;
		}
		case Float:
		{
			// double because va_list changes float to double
			// make a workaround sometime to send half the data.
			double* arg = va_arg(listPointer, double*);
			double tempValue;
			tempValue = *((double*)(&data[index]));
			*arg = *((double*)(&data[index]));
			index += sizeof(double);
			break;
		}
		case Char:
		{
			char* arg = va_arg(listPointer, char*);
			*arg = *((char*)(&data[index]));
			index += sizeof(char);
			break;
		}
		case messageID:
		{
			MessageIdentifier* arg = va_arg(listPointer, MessageIdentifier*);
			*arg = *((MessageIdentifier*)(&data[index]));
			index += sizeof(MessageIdentifier);
			break;
		}
		default:
			printf("INVALID ARGUMENT on deSerialization!\n");
			assert(0 && "DeSerialization failed");
			break;
		}
	}
	va_end(listPointer);

}
inline MessageIdentifier getID(unsigned char* data)
{
	return *((MessageIdentifier*)(&data[0]));
}

inline unsigned char* reserveSpace(DataType dataTypes[], size_t argumentAmount, size_t &size)
{
	unsigned char* data;
	size = 0;
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
		case messageID:
			size += sizeof(messageID);
			break;
		default:
			printf("INVALID ARGUMENT on deSerialization!\n");
			assert(0 && "reserving space failed");
			break;
		}

	}
	return data = (unsigned char*)malloc(size);
}