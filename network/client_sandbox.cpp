#if defined(ARENA_CLIENT)
#include "client_sandbox.h"
#include "MessageIdentifiers.h"
#include "SFML\System\Time.hpp"
#include "SFML\System\Clock.hpp"
#include <SFML/Graphics.hpp>


void Client::start(char* address, unsigned port)
{
	initializeENet();
	m_client = createENetClient();
	connect(address, port);

	sf::Clock deltaTime;
	sf::RenderWindow window(sf::VideoMode(1000,1000), "Networktest");
	sf::RectangleShape rectangle;
	rectangle.setSize(sf::Vector2f(32, 128));
	rectangle.setFillColor(sf::Color::Green);
	rectangle.setPosition(50, 50);
	m_myId = UINT32_MAX;

	while (m_myId == UINT32_MAX)
	{
		checkEvent();
	}

	while (true)
	{
		sf::Event event;
		window.pollEvent(event);
		if (event.type == sf::Event::Closed)
			window.close();
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
			m_gladiatorVector[m_myId].m_movedir_x = -3;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
			m_gladiatorVector[m_myId].m_movedir_x = 3;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
			m_gladiatorVector[m_myId].m_movedir_y = -10;

		checkEvent();

		window.clear();
		
		for(unsigned i = 0; i < m_gladiatorVector.size(); i++)
		{ 
			rectangle.setPosition(m_gladiatorVector[i].m_position_x, m_gladiatorVector[i].m_position_y);
			window.draw(rectangle);
		}
		window.display();

		

		if (deltaTime.getElapsedTime() > sf::milliseconds(200))
		{
			if (m_gladiatorVector[m_myId].m_movedir_x != 0 ||
				m_gladiatorVector[m_myId].m_movedir_y != 0)
			{
				size_t size;
				unsigned char* data = createMovePacket(size, m_gladiatorVector[m_myId].m_movedir_x,
					m_gladiatorVector[m_myId].m_movedir_y);
				sendPacket(data, size);
				m_gladiatorVector[m_myId].m_movedir_x = 0;
				m_gladiatorVector[m_myId].m_movedir_y = 0;
			}

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
	enet_host_service(m_client, &EEvent,0);

	while (EEvent.type != ENET_EVENT_TYPE_NONE)
	{
		switch (EEvent.type)
		{
			case ENET_EVENT_TYPE_CONNECT:
				printf("Somebody connected here?",
					EEvent.peer->address.host,
					EEvent.peer->address.port);
				EEvent.peer->data = "Host";
				break;

			case ENET_EVENT_TYPE_RECEIVE:
			{
				// Use data on destroy packet
				MessageIdentifiers id = getID(EEvent.packet->data);
				switch(id)
					{ 
					case Update:
						openUpdatePackage(EEvent.packet->data);
						break;
					case Start:
						openStartPackage(EEvent.packet->data);
						break;
					}
				enet_packet_destroy(EEvent.packet);
				break;
			}
			case ENET_EVENT_TYPE_DISCONNECT:
				printf("%s disconnected.\n", EEvent.peer->data);
				EEvent.peer->data = NULL;
		}
		enet_host_service(m_client, &EEvent, 0);
	}
	
}

MessageIdentifiers Client::getID(unsigned char* data)
{
	return *((MessageIdentifiers*)(&data[0]));
}

void Client::openUpdatePackage(unsigned char* data)
{
	size_t index = sizeof(MessageIdentifiers);

	for(unsigned i = 0; i < m_gladiatorVector.size(); i++)
	{
		m_gladiatorVector[i].m_position_x = *((float*)(&data[index]));
		index += sizeof(float);

		m_gladiatorVector[i].m_position_y = *((float*)(&data[index]));
		index += sizeof(float);

		m_gladiatorVector[i].m_velocity_x = *((float*)(&data[index]));
		index += sizeof(float);

		m_gladiatorVector[i].m_velocity_y = *((float*)(&data[index]));
		index += sizeof(float);

		m_gladiatorVector[i].m_rotation = *((float*)(&data[index]));
		index += sizeof(float);
	}
}

void Client::openStartPackage(unsigned char* data)
{
	// This should be called only once.
	size_t index = sizeof(MessageIdentifiers);

	m_myId = *((unsigned*)(&data[index]));
	index += sizeof(unsigned);
	
	unsigned playerAmount = *((unsigned*)(&data[index]));
	printf("Received startpackage my id: %u, playeramount: %u", m_myId, playerAmount);

	for (unsigned i = 0; i < playerAmount; i++)
	{
		Gladiator glad;
		glad.id = i;
		glad.m_velocity_x = 0;
		glad.m_velocity_y = 0;
		glad.m_position_x = i*50;
		glad.m_position_y = 50;
		glad.m_movedir_x = 2;
		glad.m_movedir_y = -0.5;
		glad.m_rotation = 13;
		m_gladiatorVector.push_back(glad);
	}
	
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

#endif