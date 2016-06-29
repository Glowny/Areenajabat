#if defined(ARENA_CLIENT)
#include "client_sandbox.h"
#include "Enumerations.h"
#include "SFML\System\Time.hpp"
#include "SFML\System\Clock.hpp"
#include <SFML/Graphics.hpp>
#include "Serializer.h"

void Client::start(char* address, unsigned port)
{
	initializeENet();
	m_client = createENetClient();
	connect(address, port);

	sf::Clock deltaTime;
	sf::Clock physicsClock;
	sf::RenderWindow window(sf::VideoMode(1100,1000), "Networktest");
	sf::RectangleShape rectangle;
	rectangle.setSize(sf::Vector2f(32, 128));
	rectangle.setFillColor(sf::Color::Green);
	rectangle.setPosition(50, 50);
	m_myId = UINT32_MAX;

	while (m_myId == UINT32_MAX)
	{
		checkEvent();
	}
	ennakointi = 0;
	while (true)
	{
		sf::Event event;
		window.pollEvent(event);
		if (event.type == sf::Event::Closed)
			window.close();
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
			m_gladiatorVector[m_myId].m_movedir_x = -3.0f;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
			m_gladiatorVector[m_myId].m_movedir_x = 3.0f;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
			m_gladiatorVector[m_myId].m_movedir_y = -10.0f;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::T))
			ennakointi += 0.000001;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::G))
			ennakointi -= 0.000001;
		if (physicsClock.getElapsedTime() > sf::milliseconds(16.0f))
		{ 

			for (unsigned i = 0; i < m_gladiatorVector.size(); i++)
			{
				m_gladiatorVector[i].m_position_x += m_gladiatorVector[i].m_velocity_y * ennakointi;
				m_gladiatorVector[i].m_position_y += m_gladiatorVector[i].m_velocity_x * ennakointi;
			}
			physicsClock.restart();
		}
		checkEvent();

		window.clear();
		
		for(unsigned i = 0; i < m_gladiatorVector.size(); i++)
		{ 
			rectangle.setPosition(m_gladiatorVector[i].m_position_x, m_gladiatorVector[i].m_position_y);
			window.draw(rectangle);
			for (unsigned i = 0; i < m_points.size(); i++)
			{
				for (unsigned j = 0; j < m_points[i].m_size; j++)
				{
					window.draw(&m_points[i].platform[j], 2, sf::Lines);
				}
			}
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
				deltaTime.restart();
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
				MessageIdentifier id = getID(EEvent.packet->data);
				switch(id)
					{ 
					case Update:
						openUpdatePackage(EEvent.packet->data);
						break;
					case Start:
						openStartPackage(EEvent.packet->data);
						break;
					case PlatformData:
						openPlatformPackage(EEvent.packet->data);
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

MessageIdentifier Client::getID(unsigned char* data)
{
	return *((MessageIdentifier*)(&data[0]));
}

void Client::openUpdatePackage(unsigned char* data)
{
	size_t index = sizeof(MessageIdentifier);
	DataType dataTypes[5]{ Float, Float, Float, Float, Float };
	
	for(unsigned i = 0; i < m_gladiatorVector.size(); i++)
	{
		double tempx, tempy;
		tempx = m_gladiatorVector[i].m_position_x; tempy = m_gladiatorVector[i].m_position_y;
		deSerializeWithIndex(data, index, dataTypes, 5,
			&m_gladiatorVector[i].m_position_x, &m_gladiatorVector[i].m_position_y,
			&m_gladiatorVector[i].m_velocity_x, &m_gladiatorVector[i].m_velocity_y,
			&m_gladiatorVector[i].m_rotation);
		printf("position difference: %f, %f\n velocity: %f, %f\n ennakointi: %f\n", m_gladiatorVector[i].m_position_x-tempx, m_gladiatorVector[i].m_position_y-tempy,
									m_gladiatorVector[i].m_velocity_x, m_gladiatorVector[i].m_velocity_y, ennakointi);
	}

}
void Client::openPlatformPackage(unsigned char* data)
{
	DataType dataTypes[1]{ unsignedInt };
	size_t index = sizeof(MessageIdentifier);

	unsigned pointAmount;
	deSerializeWithIndex(data, index, dataTypes, 1, &pointAmount);
	
	PlatformPoints points;
	points.platform = new sf::Vertex[pointAmount];
	points.m_size = pointAmount;

	DataType pointTypes[2]{ Float,Float };
	double tempx, tempy;
	for (unsigned i = 0; i < pointAmount; i++)
	{
		deSerializeWithIndex(data, index, pointTypes, 2, 
			&tempx, &tempy);
		points.platform[i]= sf::Vertex(sf::Vector2f(float(tempx), float(tempy)));
	}

	m_points.push_back(points);
	
}
void Client::openStartPackage(unsigned char* data)
{
	// This should be called only once.
	size_t index = sizeof(MessageIdentifier);

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

unsigned char* Client::createMovePacket(size_t &size, double movedir_x,
	double movedir_y)
{
	size = sizeof(size_t) + sizeof(double) * 2;
	size_t index = 0;
	unsigned char* data = (unsigned char*)malloc(size);

	*((MessageIdentifier*)(&data[index])) = ClientMove;
	index += sizeof(MessageIdentifier);

	*((double*)(&data[index])) = movedir_x;
	index += sizeof(double);

	*((double*)(&data[index])) = movedir_y;

	return data;
}

#endif