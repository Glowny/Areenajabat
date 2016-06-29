#if defined(ARENA_CLIENT)
#include "client_sandbox.h"

void Client::start(char* address, unsigned port)
{
	m_network.setMessageQueue(&m_messageQueue);
	m_network.connectServer(address, port);

	m_networkClock.restart();
	m_physicsClock.restart();

	m_window = new sf::RenderWindow (sf::VideoMode(1100,1000), "Networktest");

	m_rectangle.setSize(sf::Vector2f(32, 128));
	m_rectangle.setFillColor(sf::Color::Green);
	m_rectangle.setPosition(50, 50);

	m_myId = UINT32_MAX;

	while (m_myId == UINT32_MAX)
	{
		m_network.checkEvent();
		handleMessages();
	}

	while (true)
	{
		getInput();
		m_network.checkEvent();
		handleMessages();
		updatePhysics();
		sendData();
		draw();
	}

}
void Client::handleMessages()
{
	while (m_messageQueue.size() != 0)
	{
		handleMessage(m_messageQueue.front());
		//free(m_messageQueue.front());
		m_messageQueue.pop();
	}
}
void Client::handleMessage(unsigned char* data)
{
	MessageIdentifier messageId = getMessageID(data);

	switch (messageId)
	{
	case Update:
		openUpdatePacket(data, m_gladiatorVector);
		break;
	case Start:
	{
		unsigned playerAmount;
		openSetupPacket(data, playerAmount, m_myId);
		for (unsigned i = 0; i < playerAmount; i++)
		{
			m_gladiatorVector.push_back(GladiatorData());
		}
		break;
	}
	case PlatformData:
		openPlatformPacket(data, m_points);
		for (unsigned i = 0; i < m_points.size(); i++)
		{
			std::vector<sf::Vertex> drawablePlatform;
			for (unsigned j = 0; j < m_points[i].points.size(); j++)
			{
				sf::Vertex vertex(sf::Vector2f(m_points[i].points[j].x, m_points[i].points[j].y));
				drawablePlatform.push_back(vertex);
			}
			m_vertexes.push_back(drawablePlatform);
		}
		break;
	default:
		break;
	}

	free(data);
}

void Client::draw()
{
	m_window->clear();
	for (unsigned i = 0; i < m_gladiatorVector.size(); i++)
	{
		m_rectangle.setPosition(m_gladiatorVector[i].position.x, m_gladiatorVector[i].position.y);
		m_window->draw(m_rectangle);
		for (unsigned i = 0; i < m_vertexes.size(); i++)
		{
			for (unsigned j = 0; j < m_vertexes[i].size(); j++)
			{

				m_window->draw(&m_vertexes[i][j], 2, sf::Lines);
			}
		}
	}
	m_window->display();

}

void Client::getInput()
{
	sf::Event event;
	m_window->pollEvent(event);
	if (event.type == sf::Event::Closed)
		m_window->close();
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		m_movedir.x = -3.0f;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		m_movedir.x = 3.0f;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		m_movedir.y = -10.0f;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		m_movedir.y = 10.0f;

}
void Client::sendData()
{
	if (m_networkClock.getElapsedTime() > sf::milliseconds(200))
	{
		if (m_movedir.x != 0 ||
			m_movedir.y != 0)
		{
			size_t size;
			unsigned char* data = createMovePacket(size, m_movedir);
			m_network.sendPacket(data, size);
			m_movedir.x = 0;
			m_movedir.y = 0;
			m_networkClock.restart();
		}

	}
}

void Client::updatePhysics()
{
	if (m_physicsClock.getElapsedTime() > sf::milliseconds(1.6f))
	{
		for (unsigned i = 0; i < m_gladiatorVector.size(); i++)
		{
			// not very good
			m_gladiatorVector[i].position.x += m_gladiatorVector[i].velocity.y * 0.00024;
			m_gladiatorVector[i].position.y += m_gladiatorVector[i].velocity.x * 0.00024;
		}
		m_physicsClock.restart();
	}
}
#endif