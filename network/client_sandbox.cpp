#if defined(ARENA_CLIENT)
#include "client_sandbox.h"

void Client::start(char* address, unsigned port)
{
	m_network.setMessageQueue(&m_messageQueueIn);
	m_network.connectServer(address, port);

	m_networkClock.restart();
	m_physicsClock.restart();

	m_window = new sf::RenderWindow (sf::VideoMode(1100,1000), "Networktest");
	m_view.reset(sf::FloatRect(0, 0, 1100, 1000));
	m_view.setCenter(sf::Vector2f(550, 500));
	m_window->setView(m_view);
	m_rectangle.setSize(sf::Vector2f(32, 32));
	m_rectangle.setFillColor(sf::Color::Green);
	m_rectangle.setPosition(200, 50);

	m_bulletRectangle.setSize(sf::Vector2f(20, 20));
	m_bulletRectangle.setFillColor(sf::Color::White);

	noMoreBullets = false;

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
	while (m_messageQueueIn.size() != 0)
	{
		handleMessage(m_messageQueueIn.front());
		m_messageQueueIn.pop();
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
	case CreateBullet:
	{
		std::vector<BulletOutputData> outputBulletVector;
		openBulletOutputPacket(data, outputBulletVector);
		for (unsigned i = 0; i < outputBulletVector.size(); i++)
		{
			LiveBullet bullet;
			bullet.position = outputBulletVector[i].position;
			bullet.velocity = outputBulletVector[i].velocity;
			bullet.m_rectangle = &m_bulletRectangle;
			m_liveBulletVector.push_back(bullet);
		}
	}
	default:
		break;
	}

	free(data);
}

void Client::draw()
{
	m_window->setView(m_view);
	m_window->clear();
	for (unsigned i = 0; i < m_gladiatorVector.size(); i++)
	{
		// draw platforms
		for (unsigned i = 0; i < m_vertexes.size(); i++)
		{
			
			m_window->draw(&m_vertexes[i][0], m_vertexes[i].size(), sf::LinesStrip);

		}
		// draw player
		m_rectangle.setPosition(m_gladiatorVector[i].position.x, m_gladiatorVector[i].position.y);
		m_window->draw(m_rectangle);
		// draw bullets

		for (unsigned i = 0; i < m_liveBulletVector.size(); i++)
		{
			m_liveBulletVector[i].m_rectangle->setPosition(m_liveBulletVector[i].position.x, m_liveBulletVector[i].position.y);
			m_window->draw(*m_liveBulletVector[i].m_rectangle);

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
	if (event.type == sf::Event::KeyPressed)
	{
		if (event.key.code == sf::Keyboard::T)
		{

			BulletInputData bullet;
			bullet.bulletType = UMP45;	bullet.rotation = 0;
			if (noMoreBullets == false)
			{
				m_bulletVector.push_back(bullet);
				noMoreBullets = true;
			}
		}
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		m_movedir.x = -3.0f;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		m_movedir.x = 3.0f;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		m_movedir.y = -10.0f;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		m_movedir.y = 10.0f;

	// Camera
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
		m_view.move(-0.1, 0);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
		m_view.move(0.1, 0);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
		m_view.move(0, -0.1);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
		m_view.move(0, 0.1);


	if (event.type == sf::Event::Closed)
		m_window->close();
	

}
void Client::sendData()
{
	if (m_networkClock.getElapsedTime() > sf::milliseconds(16))
	{
		// Send player movement direction.
		if (m_movedir.x != 0 ||
			m_movedir.y != 0)
		{ 
			size_t movePacketSize;
			unsigned char* moveData = createMovePacket(movePacketSize, m_movedir);
			m_network.sendPacket(moveData, movePacketSize);
			m_movedir.x = 0;
			m_movedir.y = 0;
		}
		// Send all bullets created during loop.
		if (m_bulletVector.size() != 0)
		{
			size_t bulletPacketSize;
			unsigned char* bulletData = createBulletInputPacket(bulletPacketSize, m_bulletVector);
			m_network.sendPacket(bulletData, bulletPacketSize);
			m_bulletVector.clear();
			noMoreBullets = false;
		}
		// Send created packets.
		m_network.sendMessages();
	
		m_networkClock.restart();
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
		for (unsigned i = 0; i < m_liveBulletVector.size(); i++)
		{
			m_liveBulletVector[i].position += glm::vec2(m_liveBulletVector[i].velocity.x *0.01, m_liveBulletVector[i].velocity.y *0.01);
		}
		m_physicsClock.restart();
	}
}
#endif