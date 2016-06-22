#include "client_sandbox.h"
#include "MessageIdentifiers.h"
void Client::start(char* address, unsigned port)
{
	initializeENet();
	m_client = createENetClient();
	connect(address, port);

	m_gladiator.id = 0;
	m_gladiator.m_velocity_x = 0;
	m_gladiator.m_velocity_y = 0;
	m_gladiator.m_position_x = 50;
	m_gladiator.m_position_y = 50;
	m_gladiator.m_movedir_x = 1;
	m_gladiator.m_movedir_y = -1;
	m_gladiator.m_rotation = 13;

	float deltaTime = 0;
	while (true)
	{
		checkEvent();
		deltaTime += 0.05;

		if (deltaTime > 100000)
		{ 

			printf("position: (%f, %f), velocity(%f, %f)\n", m_gladiator.m_position_x, m_gladiator.m_position_y,
				m_gladiator.m_velocity_x, m_gladiator.m_velocity_y);
			printf("Sending velocity packet \n");
			size_t size;
			unsigned char* data = createMovePacket(size, m_gladiator.m_movedir_x,
													m_gladiator.m_movedir_y);
			sendPacket(data, size );
			deltaTime = 0;
		}
	}

}

void Client::initializeENet()
{
	if (enet_initialize() != 0)
	{
		fprintf(stderr, "An error occured while intializing ENET. \n");
		abort();
	}
}

ENetHost* Client::createENetClient()
{
	ENetHost* client;
	client = enet_host_create(NULL /* create a client host */,
		1 /* only allow 1 outgoing connection */,
		2 /* allow up 2 channels to be used, 0 and 1 */,
		57600 / 8 /* 56K modem with 56 Kbps downstream bandwidth */,
		14400 / 8 /* 56K modem with 14 Kbps upstream bandwidth */);

	if (client == NULL)
	{
		fprintf(stderr, "An error occured while rying to create an ENEt client host .\n");
		abort();
	}
	return client;
}
void Client::checkEvent()
{
	ENetEvent EEvent;
	enet_host_service(m_client, &EEvent, 0);

	switch (EEvent.type)
	{
	case ENET_EVENT_TYPE_CONNECT:
		printf("Somebody connected here?",
			EEvent.peer->address.host,
			EEvent.peer->address.port);
		EEvent.peer->data = "Host";
		break;

	case ENET_EVENT_TYPE_RECEIVE:

		// Use data on destroy packet
		openUpdatePackage(EEvent.packet->data);
		enet_packet_destroy(EEvent.packet);
		break;

	case ENET_EVENT_TYPE_DISCONNECT:
		printf("%s disconnected.\n", EEvent.peer->data);
		EEvent.peer->data = NULL;
	}

}

void Client::openUpdatePackage(unsigned char* data)
{
	size_t index = 0;
	// open id, should be done before this because this calls the function for opening
	// rest of the package.
	MessageIdentifiers id;
	id = *((MessageIdentifiers*)(&data[index]));
	index += sizeof(MessageIdentifiers);

	// Get bullet array size;
	size_t bulletAmount = *((size_t*)(&data[index]));
	index += sizeof(size_t);

	m_gladiator.m_position_x = *((float*)(&data[index]));
	index += sizeof(float);

	m_gladiator.m_position_y = *((float*)(&data[index]));
	index += sizeof(float);

	m_gladiator.m_velocity_x = *((float*)(&data[index]));
	index += sizeof(float);

	m_gladiator.m_velocity_y = *((float*)(&data[index]));
	index += sizeof(float);

	m_gladiator.m_rotation = *((float*)(&data[index]));

}

void Client::connect(char* address, unsigned port)
{
	ENetAddress eAddress;
	enet_address_set_host(&eAddress, address);
	eAddress.port = port;

	m_peer = enet_host_connect(m_client, &eAddress, 2, 0);
	if (m_peer == NULL)
	{
		fprintf(stderr,
			"No available peers for initiating an ENet connection.\n");
		abort();
	}

	if (enet_host_service(m_client, &m_eEvent, 5000) > 0 &&
		m_eEvent.type == ENET_EVENT_TYPE_CONNECT)
	{
		printf("Connection to %s,: %u succeeded", address, &port);
	}
	else
	{
		enet_peer_reset(m_peer);
		printf("Connection to %s,: %u failed", address, &port);
	}
}

void Client::sendPacket(unsigned char* data, unsigned size)
{
	ENetPacket *packet = enet_packet_create(data,
		size,
		ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(m_peer, 0, packet);
	// When all packets are ready to send use
	enet_host_flush(m_client);
}

unsigned char* Client::createMovePacket(size_t &size, float movedir_x,
										float movedir_y)
{
	size = sizeof(size_t) + sizeof(float) * 2;
	size_t index = 0;
	unsigned char* data = (unsigned char*)malloc(size);

	*((MessageIdentifiers*)(&data[index])) = ClientFeedback;
	index += sizeof(MessageIdentifiers);
	*((float*)(&data[index])) = movedir_x;
	index += sizeof(float);
	*((float*)(&data[index])) = movedir_y;

	return data;
}