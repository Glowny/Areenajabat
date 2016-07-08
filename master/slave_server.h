#pragma once

#include "common\forward_declare.h"
#include "common\types.h"

FORWARD_DECLARE(FORWARD_DECLARE_TYPE_CLASS, Server)

namespace arena
{
	class SlaveServer final
	{
	public:
		SlaveServer(const uint32 address, const uint32 port);

		void initialize();

		bool start();
		bool stop();

		~SlaveServer();
	private:
		Server* const m_instance;

		const uint32 m_address;
		const uint32 m_port;
	};
}