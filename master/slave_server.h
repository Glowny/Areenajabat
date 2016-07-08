#pragma once

#include "common\forward_declare.h"
#include "common\types.h"

FORWARD_DECLARE(FORWARD_DECLARE_TYPE_CLASS, Server)

/*
	Wraps the server instances that the master manages.
	One machine can run multiple instances of game
	server.
*/

// TODO: start working with slave after the planning session 
//		 that happens at the evening of 8.7.2016


//namespace arena
//{
//	class SlaveServer final
//	{
//	public:
//		SlaveServer(const uint32 address, const uint32 port);
//
//		void initialize();
//
//		bool start();
//		bool stop();
//
//		~SlaveServer();
//	private:
//		Server* const m_instance;
//
//		const uint32 m_address;
//		const uint32 m_port;
//	};
//}