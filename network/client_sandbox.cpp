#if defined(ARENA_CLIENT)
#include "client_sandbox.h"

void Client::start(char* address, unsigned port)
{
	font.loadFromFile("asd.ttf");
	hpText.setFont(font);
	hpText.setColor(sf::Color::Red);
	hpText.setCharacterSize(40.0f);
	m_network.setMessageQueue(&m_messageQueueIn);
	m_network.connectServer(address, port);

	m_timerClock.restart();
	m_networkClock.restart();
	m_physicsClock.restart();

	m_window = new sf::RenderWindow (sf::VideoMode(1100,1000), "Networktest");
	m_view.reset(sf::FloatRect(0, 0, 1100, 1000));
	m_view.zoom(1.5);
	m_view.setCenter(sf::Vector2f(550, 500));
	m_window->setView(m_view);
	m_rectangle.setSize(sf::Vector2f(0.4f*100.0f, 1.2f*100.0f));
	m_rectangle.setOrigin(sf::Vector2f(20.0f, 60.0f));
	m_rectangle.setFillColor(sf::Color::Green);
	m_rectangle.setPosition(200, 50);
	
	m_bulletRectangle.setSize(sf::Vector2f(5, 5));
	m_bulletRectangle.setFillColor(sf::Color::White);

	aimAngle = 0.03f;

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
		updateGameplay();
		sendData();
		draw();
		m_timerClock.restart();
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
			GladiatorData data;
			data.alive = true;
			data.hitPoints = 100;
			m_gladiatorVector.push_back(data);
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
		break;
	}
	case Hit:
	{
		m_liveBulletVector.clear();
		BulletHit hit;
		hit.lifeTime = 4;
		openHitPacket(data, hit.position);
		m_bulletHitVector.push_back(hit);
		break;
	}
	case BulletUpdate:
	{
		m_liveBulletVector.clear();
		std::vector<glm::vec2> bulletPositions;
		std::vector<glm::vec2> bulletVelocities;
		openBulletUpdatePacket(data, bulletPositions, bulletVelocities);
		
		for(unsigned i = 0; i < bulletPositions.size(); i++)
		{ 
			LiveBullet bullet;
			bullet.position.x = bulletPositions[i].x;
			bullet.position.y = bulletPositions[i].y;
			bullet.velocity.x = bulletVelocities[i].x;
			bullet.velocity.y = bulletVelocities[i].y;
			m_liveBulletVector.push_back(bullet);
		}
		break;
	}
	case PlayerDamage:
	{
		unsigned damagedPlayer, damageAmount;
		openPlayerDamagePacket(data, damagedPlayer, damageAmount);
		m_gladiatorVector[damagedPlayer].hitPoints -= 10;
		break;
	}
	case PlayerKill:
	{
		unsigned killedPlayer;
		openPlayerKillPacket(data, killedPlayer);
		m_gladiatorVector[killedPlayer].alive = false;
		break;
	}
	case PlayerRespawn:
	{
		unsigned respawnPlayer;
		openPlayerRespawnPacket(data, respawnPlayer);
		m_gladiatorVector[respawnPlayer].alive = true;
		m_gladiatorVector[respawnPlayer].hitPoints = 100;
		break;
	}

	default:
		break;
	}

	free(data);
}

sf::Color playerColors[10]
{
	sf::Color::Color(0,	0,	255,255),
	sf::Color::Color(0,	255,255,255),
	sf::Color::Color(0,	255,0,255),
	sf::Color::Color(255,0,	255,255),
	sf::Color::Color(255,0,	0,255),
	sf::Color::Color(255,255,0,255),
	sf::Color::Color(122,255,122,255),
	sf::Color::Color(0,122,255,255),
	sf::Color::Color(112,255,0,255),
	sf::Color::Color(0,112,0,255),
};

void Client::draw()
{
	sf::Vector2f playerPosition;
	playerPosition.x = m_gladiatorVector[m_myId].position.x;
	playerPosition.y = m_gladiatorVector[m_myId].position.y;
	m_view.setCenter(playerPosition);
	m_window->setView(m_view);
	m_window->clear();
	// draw platforms
	for (unsigned i = 0; i < m_vertexes.size(); i++)
	{
		m_window->draw(&m_vertexes[i][0], m_vertexes[i].size(), sf::LinesStrip);
	}

	for (unsigned i = 0; i < m_gladiatorVector.size(); i++)
	{
		// draw player
		m_rectangle.setPosition(m_gladiatorVector[i].position.x, m_gladiatorVector[i].position.y);
		m_rectangle.setFillColor(playerColors[i]);
		m_window->draw(m_rectangle);
		if (m_gladiatorVector[i].alive == true)
		{
			hpText.setString(std::to_string(m_gladiatorVector[i].hitPoints));
			hpText.setPosition(m_rectangle.getPosition());
		}
		else
		{ 
			hpText.setString("DEAD");
			hpText.setPosition(m_rectangle.getPosition());
		}
		m_window->draw(hpText);
	}
	for (unsigned i = 0; i < m_liveBulletVector.size(); i++)
	{
		m_bulletRectangle.setPosition(m_liveBulletVector[i].position.x, m_liveBulletVector[i].position.y);
		m_bulletRectangle.setFillColor(sf::Color::Yellow);
		m_window->draw(m_bulletRectangle);
	}
	
	// draw hits
	for (unsigned i = 0; i < m_bulletHitVector.size(); i++)
	{
		m_bulletRectangle.setPosition(m_bulletHitVector[i].position.x, m_bulletHitVector[i].position.y);
		m_bulletRectangle.setFillColor(sf::Color::Red);
		m_window->draw(m_bulletRectangle);
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
			bullet.bulletType = UMP45;	bullet.rotation = aimAngle;
			printf("Aim angle: %f\n", aimAngle);
			if (noMoreBullets == false)
			{
				m_bulletVector.push_back(bullet);
				noMoreBullets = true;
			}
		}
		if (event.key.code == sf::Keyboard::W)
		{
			m_movedir.y = -1.0f;
		}
		//if (event.key.code == sf::Keyboard::A)
		//{
		//	
		//}
		//if (event.key.code == sf::Keyboard::D)
		//{
		//	
		//}
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		m_movedir.x = -1.0f;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		m_movedir.x = 1.0f;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
	{
		aimAngle += 0.0007f;
		if (aimAngle > 6.28f)
			aimAngle = 0.03f;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::X))
	{
		aimAngle -= 0.0007f;
		if (aimAngle < 0.0f)
			aimAngle = 6.26f;
	}


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
		//for (unsigned i = 0; i < m_liveBulletVector.size(); i++)
		//{
		//	m_liveBulletVector[i].position.x = m_liveBulletVector[i].position.x + m_liveBulletVector[i].velocity.x * 0.00024;
		//	m_liveBulletVector[i].position.y = m_liveBulletVector[i].position.y + m_liveBulletVector[i].velocity.y * 0.00024;
		//}

	m_physicsClock.restart();
	}
}

void Client::updateGameplay()
{
	for(unsigned i = 0; i < m_bulletHitVector.size(); i++)
	{ 
		float time = m_timerClock.getElapsedTime().asSeconds();
		m_bulletHitVector[i].currentTime += time;
		if (m_bulletHitVector[i].lifeTime < m_bulletHitVector[i].currentTime)
		{
			m_bulletHitVector.erase(m_bulletHitVector.begin()+i);
			i += -1;
		}
	}
}

#endif