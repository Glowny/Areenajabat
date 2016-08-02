#pragma once

#include "../arena_types.h"

#include <vector>

/*
	Basic RTTI support that has typenames and IDs.
*/

namespace arena
{
	// Class to contain basic RTTI data.
	class Type final 
	{
	public:
		const char* const	m_typename;
		const uint32		m_typeID;
		const uint32		m_size;

		Type(const char* const tname, const uint32 size);
		
		~Type() = default;

		bool operator ==(const Type& lhs) const;
		bool operator !=(const Type& lhs) const;
	private:
		static uint32 s_IDGenerator;
	};
}

/*
    Use this macro at the beginning of an
    class declaration to create new RTTI type.
*/

#define DEFINE_RTTI_SUB_TYPE(__typename__ ) public: \
												static Type s_type##__typename__; \
                                                Type getType() override { return s_type##__typename__; }

#define DEFINE_RTTI_SUPER_TYPE(__typename__)   public:                          \
                                               static Type s_type##__typename__; \
											   public: \
												   virtual Type getType() { return s_type##__typename__; } \
											   

#define REGISTER_RTTI_SUB_TYPE(__typename__) Type __typename__::s_type##__typename__ = Type(#__typename__, sizeof(__typename__)); 

#define INITIALIZE_RTTI_SUPER_TYPE(__typename__) REGISTER_RTTI_SUB_TYPE(__typename__) 											

#define TYPEOF(__typename__)				__typename__::s_type##__typename__
#define TYPENAME(__typename__)				__typename__::s_type##__typename__.m_name
#define TYPEID(__typename__)				__typename__::s_type##__typename__.m_typeID

/*
	TODO: add inheritance (base class) checks if needed.
*/