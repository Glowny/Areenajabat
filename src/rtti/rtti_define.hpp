#pragma once

#include "..\arena_types.h"

/*
	Basic RTTI support that has typenames and IDs.
*/

namespace arena
{
	// Class to contain basic RTTI data.
	class RTTIData final {
	public:
		const String	m_typename;
		const uint64	m_typeID;

		RTTIData(const String tname);
		
		~RTTIData() = default;
	private:
		static uint64 s_IDGenerator;
	};	
}

/*
	Use this macro at the beginning of an
	class declaration to create new RTTI type.
*/

#define DEFINE_RTTI_TYPE() private: \
						      static arena::RTTIData s_rttiData; \
						   public: \
							   static uint64 RTTIGetID() const; \
							   static String RTTIGetTypename() const; \


#define REGISTER_RTTI_TYPE(__typename__) arena::RTTIData __typename__::s_rttiData = arena::RTTIData(__typename__) \
										 uint64 __typename__::RTTIGetID() const { return __typename__::s_rttiData.m_typeID; } \
										 String __typename__::RTTIGetTypename() const { return __typename__::s_rttiData.m_typename; } \

#define TYPENAMEOF(__ptr__)			__ptr__->RTTIGetTypename()
#define TYPEOF(__ptr__)				__ptr__->RTTIGetID()	

#define TYPENAMEOF(__typename__)	__typename__::RTTIGetTypename()
#define TYPEOF(__typename__)		__typename__::RTTIGetID()	