#pragma once

#include "..\arena_types.h"

#include <vector>

/*
	Basic RTTI support that has typenames and IDs.
*/

namespace arena
{
	// Class to contain basic RTTI data.
	class RTTIData final 
	{
	public:
		const String	m_typename;
		const uint32	m_typeID;
		const uint32	m_size;

		RTTIData(const String tname, const uint32 size);

		~RTTIData() = default;
	private:
		static uint32 s_IDGenerator;
	};
}

/*
	Use this macro at the beginning of an
	class declaration to create new RTTI type.
*/

#define DEFINE_RTTI_SUB_TYPE(__typename__ ) public: \
												static RTTIData s_rtti_data_##__typename__; \

#define DEFINE_RTTI_SUPER_TYPE(__typename__)   DEFINE_RTTI_SUB_TYPE(__typename__) \
											   private: \

#define REGISTER_RTTI_SUB_TYPE(__typename__) RTTIData __typename__::s_rtti_data_##__typename__ = RTTIData(#__typename__, sizeof(__typename__));

#define INITIALIZE_RTTI_SUPER_TYPE(__typename__) REGISTER_RTTI_SUB_TYPE(__typename__) 											

#define TYPEOF(__typename__)				__typename__::s_rtti_data_##__typename__
#define TYPENAME(__typename__)				__typename__::s_rtti_data_##__typename__.m_name
#define TYPEID(__typename__)				__typename__::s_rtti_data_##__typename__.m_typeID

/*
	TODO: add inheritance (base class) checks if needed.
*/