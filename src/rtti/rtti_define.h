#pragma once

#include "..\arena_types.h"

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

#define DEFINE_RTTI_TYPE  private: \
						      static RTTIData s_rttiData; \
						   public: \
							  static uint32 RTTIGetID(); \
							  static String RTTIGetTypename(); \
							  static RTTIData RTTIGetType(); \


#define REGISTER_RTTI_TYPE(__typename__) RTTIData __typename__::s_rttiData = RTTIData(#__typename__, sizeof(__typename__)); \
										 uint32 __typename__::RTTIGetID()		{	return __typename__::s_rttiData.m_typeID;		} \
										 String __typename__::RTTIGetTypename() {	return __typename__::s_rttiData.m_typename;		} \
										 RTTIData __typename__::RTTIGetType()	{	return __typename__::s_rttiData;				} \												

#define TYPENAMEOF(__ptr__)			__ptr__->RTTIGetTypename()
#define TYPEOF(__ptr__)				__ptr__->RTTIGetID()	

#define IS_OF_TYPE(__ptr__, __typename__) (__ptr__->RTTIGetID() == __typename__::RTTIGetID())