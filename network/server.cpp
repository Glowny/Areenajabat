#if defined(ARENA_SERVER)
#include "server.h"
#include <stdio.h>
#include <assert.h>
#include <bx/timer.h>

Server::Server()
{
	tempPlatformCreation();
}

Server::~Server()
{

}

void Server::start(unsigned address, unsigned port, unsigned playerAmount)
{
	m_messageQueue = new std::queue<Message>;
	m_network.startServer(m_messageQueue, address, port, playerAmount);
	
	// wait for players..
	while (m_network.getConnectedPlayerAmount() < playerAmount)
		m_network.checkEvent();

	printf("GAME STARTING!\n");

	for (unsigned i = 0; i < playerAmount; i++)
	{
		PlayerInput input;
		input.moveDir.x = 0;
		input.moveDir.y = 0;
		m_playerInputVector.push_back(input);
	}

	for (unsigned i = 0; i < playerAmount; i++)
	{
		GladiatorData glad;
		m_physics.addGladiator(glm::vec2(i*100.0f,50.0f));
		glad.rotation = 0;
		glad.position.x = 100.0f * i;
		glad.position.y = 50.0f;
		m_gladiatorVector.push_back(glad);
	}
	
	m_updateSize = sizeof(MessageIdentifier) + sizeof(float) * 5 * m_gladiatorVector.size();
	m_updateMemory = (unsigned char*)malloc(m_updateSize);
	
	for(unsigned i = 0; i < playerAmount; i++)
		{
			size_t size;
			unsigned char* data = createSetupPacket(size,playerAmount, i);
			m_network.sendPacket(data, size, i);

			// Could be broadcasted.
			size_t size2;
			unsigned char* data2 = createPlatformPacket(size2,m_platformVector);
			m_network.sendPacket(data2, size2, i);
		}

	int64_t s_last_time = bx::getHPCounter();
	float updatePhysics = 0;
	float updateNetwork = 0;

	float timeStep = 1.0f/60.0f;

	while (true)
	{
		int64_t currentTime = bx::getHPCounter();
		const int64_t time = currentTime - s_last_time;
		s_last_time = currentTime;

		const double frequency = (double)bx::getHPFrequency();

		// seconds
		float lastDeltaTime = float(time * (1.0f / frequency));
		updatePhysics += lastDeltaTime;
		updateNetwork += lastDeltaTime;

		if (updatePhysics > timeStep)
		{
			for (unsigned i = 0; i < m_playerInputVector.size(); i++)
			{
				if (m_playerInputVector[i].moveDir.x != 0 || m_playerInputVector[i].moveDir.y != 0)
				{ 
					m_physics.moveGladiator(glm::vec2(m_playerInputVector[i].moveDir.x*300000, m_playerInputVector[i].moveDir.y*300000), i);
					m_playerInputVector[i].moveDir.x = 0.0;
					m_playerInputVector[i].moveDir.y = 0.0;
				}
			}
			
			updatePhysics = 0;
			m_physics.update();

			for (unsigned i = 0; i < m_gladiatorVector.size(); i++)
			{
				m_gladiatorVector[i].position = m_physics.getGladiatorPosition(i);
				m_gladiatorVector[i].velocity = m_physics.getGladiatorVelocity(i);
			}

		}
		m_network.checkEvent();

		handleClientMessages();
		
		if ( m_bulletOutputVector.size() != 0)
		{ 
			size_t size;
			unsigned char *data = createBulletOutputPacket(size, m_bulletOutputVector);
			m_network.broadcastPacket(data, size, true);
			m_bulletOutputVector.clear();
		}
		if(updateNetwork > 0.1)
		{
			size_t size;
			unsigned char *data = createUpdatePacket(size, m_gladiatorVector, m_updateMemory);
			m_network.broadcastPacket(data, size, false);
			updateNetwork = 0;
		}
	}
}

void Server::handleClientMessages()
{
	while (m_messageQueue->size() != 0)
	{
		handleMessage(m_messageQueue->front());
		free(m_messageQueue->front().data);
		m_messageQueue->pop();
	}
}
void Server::handleMessage(Message &message)
{
	unsigned playerId = message.clientID;
	MessageIdentifier packetID = getMessageID(message.data);

	switch (packetID)
	{
	case ClientMove:
		openMovePacket(message.data, m_playerInputVector[playerId].moveDir);
		break;
	case ClientShoot:
	{
		// These are bullet creation events.
		std::vector<BulletInputData> bulletInputVector;
		openBulletInputPacket(message.data, bulletInputVector);
		createOutputBullets(bulletInputVector, playerId);
	}
		break;
	default:
		break;
	}
}

glm::vec2 radToVec(float r)
{
	return glm::vec2(cos(r), sin(r));
}

void Server::createOutputBullets(std::vector<BulletInputData> &bulletInputVector, unsigned playerId)
{
	for (unsigned i = 0; i < bulletInputVector.size(); i++)
	{
		BulletOutputData bullet;
		bullet.bulletType = bulletInputVector[i].bulletType;
		bullet.playerId = playerId;

		switch (bullet.bulletType)
		{
			case UMP45:
			{
				bullet.position.x = m_gladiatorVector[playerId].position.x;
				bullet.position.y = m_gladiatorVector[playerId].position.y;
				bullet.rotation = bulletInputVector[i].rotation;
				glm::vec2 force = radToVec(bulletInputVector[i].rotation);
				force *= 100;
				m_physics.addBullet(bullet.position, force);
				bullet.velocity.x = force.x;
				bullet.velocity.y = force.y;
			}
			case Shotgun:
			{

			}
		}
		m_bulletOutputVector.push_back(bullet);
	}
	
}

// Scary, dont look inside
void Server::tempPlatformCreation()
{

	Platform platform0;
	glm::vec2 vec;
	vec = glm::vec2(372, 421);
	platform0.points.push_back(vec);
	vec = glm::vec2(372, 494);
	platform0.points.push_back(vec);
	vec = glm::vec2(742, 491);
	platform0.points.push_back(vec);
	vec = glm::vec2(748, 418);
	platform0.points.push_back(vec);
	vec = glm::vec2(372, 424);
	platform0.points.push_back(vec);
	m_physics.createPlatform(platform0.points);
	m_platformVector.push_back(platform0);

	Platform platform1;
	vec = glm::vec2(751, 421);
	platform1.points.push_back(vec);
	vec = glm::vec2(1146, 438);
	platform1.points.push_back(vec);
	vec = glm::vec2(1143, 488);
	platform1.points.push_back(vec);
	vec = glm::vec2(1522, 485);
	platform1.points.push_back(vec);
	vec = glm::vec2(1522, 429);
	platform1.points.push_back(vec);
	vec = glm::vec2(1143, 429);
	platform1.points.push_back(vec);
	m_physics.createPlatform(platform1.points);
	m_platformVector.push_back(platform1);

	Platform platform2;
	vec = glm::vec2(1545, 432);
	platform2.points.push_back(vec);
	vec = glm::vec2(1920, 424);
	platform2.points.push_back(vec);
	vec = glm::vec2(1918, 502);
	platform2.points.push_back(vec);
	vec = glm::vec2(2249, 499);
	platform2.points.push_back(vec);
	vec = glm::vec2(2246, 418);
	platform2.points.push_back(vec);
	vec = glm::vec2(1918, 426);
	platform2.points.push_back(vec);
	m_physics.createPlatform(platform2.points);
	m_platformVector.push_back(platform2);

	Platform platform3;
	vec = glm::vec2(2249, 424);
	platform3.points.push_back(vec);
	vec = glm::vec2(2473, 477);
	platform3.points.push_back(vec);
	vec = glm::vec2(2462, 550);
	platform3.points.push_back(vec);
	vec = glm::vec2(2827, 558);
	platform3.points.push_back(vec);
	vec = glm::vec2(2821, 471);
	platform3.points.push_back(vec);
	vec = glm::vec2(2473, 477);
	platform3.points.push_back(vec);
	m_physics.createPlatform(platform3.points);
	m_platformVector.push_back(platform3);

	Platform platform4;
	vec = glm::vec2(369, 746);
	platform4.points.push_back(vec);
	vec = glm::vec2(363, 825);
	platform4.points.push_back(vec);
	vec = glm::vec2(666, 833);
	platform4.points.push_back(vec);
	vec = glm::vec2(762, 752);
	platform4.points.push_back(vec);
	vec = glm::vec2(375, 749);
	platform4.points.push_back(vec);
	m_physics.createPlatform(platform4.points);
	m_platformVector.push_back(platform4);

	Platform platform5;
	vec = glm::vec2(770, 743);
	platform5.points.push_back(vec);
	vec = glm::vec2(1141, 805);
	platform5.points.push_back(vec);
	vec = glm::vec2(1525, 808);
	platform5.points.push_back(vec);
	vec = glm::vec2(1522, 898);
	platform5.points.push_back(vec);
	vec = glm::vec2(860, 900);
	platform5.points.push_back(vec);
	m_physics.createPlatform(platform5.points);
	m_platformVector.push_back(platform5);

	Platform platform6;
	vec = glm::vec2(697, 1088);
	platform6.points.push_back(vec);
	vec = glm::vec2(1230, 1083);
	platform6.points.push_back(vec);
	m_physics.createPlatform(platform6.points);
	m_platformVector.push_back(platform6);

	Platform platform7;
	vec = glm::vec2(706, 1321);
	platform7.points.push_back(vec);
	vec = glm::vec2(1270, 1327);
	platform7.points.push_back(vec);
	m_physics.createPlatform(platform7.points);
	m_platformVector.push_back(platform7);

	Platform platform8;
	vec = glm::vec2(35, 1456);
	platform8.points.push_back(vec);
	vec = glm::vec2(436, 1450);
	platform8.points.push_back(vec);
	vec = glm::vec2(742, 1540);
	platform8.points.push_back(vec);
	vec = glm::vec2(1163, 1557);
	platform8.points.push_back(vec);
	vec = glm::vec2(1522, 1450);
	platform8.points.push_back(vec);
	vec = glm::vec2(1710, 1366);
	platform8.points.push_back(vec);
	vec = glm::vec2(1859, 1417);
	platform8.points.push_back(vec);
	vec = glm::vec2(1578, 1436);
	platform8.points.push_back(vec);
	m_physics.createPlatform(platform8.points);
	m_platformVector.push_back(platform8);

	Platform platform9;
	vec = glm::vec2(1836, 1422);
	platform9.points.push_back(vec);
	vec = glm::vec2(2748, 1400);
	platform9.points.push_back(vec);
	vec = glm::vec2(2967, 1321);
	platform9.points.push_back(vec);
	vec = glm::vec2(3225, 1565);
	platform9.points.push_back(vec);
	vec = glm::vec2(3508, 1663);
	platform9.points.push_back(vec);
	m_physics.createPlatform(platform9.points);
	m_platformVector.push_back(platform9);

	Platform platform10;
	vec = glm::vec2(3817, 1661);
	platform10.points.push_back(vec);
	vec = glm::vec2(4066, 1548);
	platform10.points.push_back(vec);
	vec = glm::vec2(4251, 1428);
	platform10.points.push_back(vec);
	vec = glm::vec2(4463, 1332);
	platform10.points.push_back(vec);
	vec = glm::vec2(4748, 1515);
	platform10.points.push_back(vec);
	vec = glm::vec2(5087, 1596);
	platform10.points.push_back(vec);
	vec = glm::vec2(5488, 1532);
	platform10.points.push_back(vec);
	vec = glm::vec2(6068, 1445);
	platform10.points.push_back(vec);
	vec = glm::vec2(6739, 1501);
	platform10.points.push_back(vec);
	vec = glm::vec2(7040, 1417);
	platform10.points.push_back(vec);
	vec = glm::vec2(7686, 1391);
	platform10.points.push_back(vec);
	m_physics.createPlatform(platform10.points);
	m_platformVector.push_back(platform10);

	Platform platform11;
	vec = glm::vec2(4735, 1293);
	platform11.points.push_back(vec);
	vec = glm::vec2(4962, 1375);
	platform11.points.push_back(vec);
	vec = glm::vec2(5155, 1366);
	platform11.points.push_back(vec);
	vec = glm::vec2(5352, 1299);
	platform11.points.push_back(vec);
	vec = glm::vec2(5405, 1254);
	platform11.points.push_back(vec);
	m_physics.createPlatform(platform11.points);
	m_platformVector.push_back(platform11);

	Platform platform12;
	vec = glm::vec2(5301, 1153);
	platform12.points.push_back(vec);
	vec = glm::vec2(5183, 1167);
	platform12.points.push_back(vec);
	vec = glm::vec2(5169, 1094);
	platform12.points.push_back(vec);
	vec = glm::vec2(5304, 1156);
	platform12.points.push_back(vec);
	m_physics.createPlatform(platform12.points);
	m_platformVector.push_back(platform12);

	Platform platform13;
	vec = glm::vec2(5068, 1069);
	platform13.points.push_back(vec);
	vec = glm::vec2(4967, 1195);
	platform13.points.push_back(vec);
	vec = glm::vec2(4822, 1164);
	platform13.points.push_back(vec);
	vec = glm::vec2(5054, 1077);
	platform13.points.push_back(vec);
	m_physics.createPlatform(platform13.points);
	m_platformVector.push_back(platform13);

	Platform platform14;
	vec = glm::vec2(7668, 1781);
	platform14.points.push_back(vec);
	vec = glm::vec2(5954, 1784);
	platform14.points.push_back(vec);
	vec = glm::vec2(5834, 1848);
	platform14.points.push_back(vec);
	vec = glm::vec2(5062, 1868);
	platform14.points.push_back(vec);
	vec = glm::vec2(4936, 1915);
	platform14.points.push_back(vec);
	vec = glm::vec2(4232, 1828);
	platform14.points.push_back(vec);
	vec = glm::vec2(4212, 1683);
	platform14.points.push_back(vec);
	vec = glm::vec2(4582, 1699);
	platform14.points.push_back(vec);
	vec = glm::vec2(4928, 1907);
	platform14.points.push_back(vec);
	m_physics.createPlatform(platform14.points);
	m_platformVector.push_back(platform14);

	Platform platform15;
	vec = glm::vec2(7671, 2039);
	platform15.points.push_back(vec);
	vec = glm::vec2(5508, 2033);
	platform15.points.push_back(vec);
	vec = glm::vec2(5146, 2120);
	platform15.points.push_back(vec);
	vec = glm::vec2(4526, 2120);
	platform15.points.push_back(vec);
	vec = glm::vec2(4085, 2053);
	platform15.points.push_back(vec);
	vec = glm::vec2(3946, 2039);
	platform15.points.push_back(vec);
	vec = glm::vec2(3823, 2092);
	platform15.points.push_back(vec);
	vec = glm::vec2(3723, 2056);
	platform15.points.push_back(vec);
	vec = glm::vec2(3658, 2073);
	platform15.points.push_back(vec);
	vec = glm::vec2(3585, 2016);
	platform15.points.push_back(vec);
	vec = glm::vec2(3490, 2039);
	platform15.points.push_back(vec);
	vec = glm::vec2(3389, 2014);
	platform15.points.push_back(vec);
	vec = glm::vec2(3293, 2047);
	platform15.points.push_back(vec);
	vec = glm::vec2(3200, 2058);
	platform15.points.push_back(vec);
	vec = glm::vec2(3086, 2109);
	platform15.points.push_back(vec);
	vec = glm::vec2(2997, 2070);
	platform15.points.push_back(vec);
	vec = glm::vec2(2788, 2123);
	platform15.points.push_back(vec);
	vec = glm::vec2(2304, 2129);
	platform15.points.push_back(vec);
	vec = glm::vec2(1964, 2047);
	platform15.points.push_back(vec);
	vec = glm::vec2(-30, 2043);
	platform15.points.push_back(vec);
	m_physics.createPlatform(platform15.points);
	m_platformVector.push_back(platform15);

	Platform platform16;
	vec = glm::vec2(-10, 1748);
	platform16.points.push_back(vec);
	vec = glm::vec2(1475, 1783);
	platform16.points.push_back(vec);
	vec = glm::vec2(1712, 1810);
	platform16.points.push_back(vec);
	vec = glm::vec2(2424, 1818);
	platform16.points.push_back(vec);
	vec = glm::vec2(2766, 1860);
	platform16.points.push_back(vec);
	vec = glm::vec2(2887, 1693);
	platform16.points.push_back(vec);
	vec = glm::vec2(2331, 1736);
	platform16.points.push_back(vec);
	vec = glm::vec2(1662, 1720);
	platform16.points.push_back(vec);
	vec = glm::vec2(1479, 1787);
	platform16.points.push_back(vec);
	m_physics.createPlatform(platform16.points);
	m_platformVector.push_back(platform16);

	Platform platform17;
	vec = glm::vec2(1984, 1242);
	platform17.points.push_back(vec);
	vec = glm::vec2(2335, 1234);
	platform17.points.push_back(vec);
	vec = glm::vec2(2766, 1273);
	platform17.points.push_back(vec);
	vec = glm::vec2(2805, 1211);
	platform17.points.push_back(vec);
	vec = glm::vec2(2389, 1176);
	platform17.points.push_back(vec);
	vec = glm::vec2(1988, 1195);
	platform17.points.push_back(vec);
	vec = glm::vec2(1988, 1242);
	platform17.points.push_back(vec);
	m_physics.createPlatform(platform17.points);
	m_platformVector.push_back(platform17);

	Platform platform18;
	vec = glm::vec2(1903, 697);
	platform18.points.push_back(vec);
	vec = glm::vec2(2284, 717);
	platform18.points.push_back(vec);
	vec = glm::vec2(2288, 791);
	platform18.points.push_back(vec);
	vec = glm::vec2(1903, 771);
	platform18.points.push_back(vec);
	vec = glm::vec2(1914, 709);
	platform18.points.push_back(vec);
	m_physics.createPlatform(platform18.points);
	m_platformVector.push_back(platform18);

	Platform platform19;
	vec = glm::vec2(2288, 705);
	platform19.points.push_back(vec);
	vec = glm::vec2(2471, 775);
	platform19.points.push_back(vec);
	vec = glm::vec2(2459, 841);
	platform19.points.push_back(vec);
	vec = glm::vec2(2817, 834);
	platform19.points.push_back(vec);
	vec = glm::vec2(2817, 760);
	platform19.points.push_back(vec);
	vec = glm::vec2(2459, 767);
	platform19.points.push_back(vec);
	m_physics.createPlatform(platform19.points);
	m_platformVector.push_back(platform19);

	Platform platform20;
	vec = glm::vec2(3381, 114);
	platform20.points.push_back(vec);
	vec = glm::vec2(3525, 180);
	platform20.points.push_back(vec);
	vec = glm::vec2(3665, 184);
	platform20.points.push_back(vec);
	m_physics.createPlatform(platform20.points);
	m_platformVector.push_back(platform20);

	Platform platform21;
	vec = glm::vec2(3778, 180);
	platform21.points.push_back(vec);
	vec = glm::vec2(3941, 122);
	platform21.points.push_back(vec);
	m_physics.createPlatform(platform21.points);
	m_platformVector.push_back(platform21);

	Platform platform22;
	vec = glm::vec2(3509, 207);
	platform22.points.push_back(vec);
	vec = glm::vec2(3521, 378);
	platform22.points.push_back(vec);
	m_physics.createPlatform(platform22.points);
	m_platformVector.push_back(platform22);

	Platform platform23;
	vec = glm::vec2(3521, 468);
	platform23.points.push_back(vec);
	vec = glm::vec2(3521, 666);
	platform23.points.push_back(vec);
	m_physics.createPlatform(platform23.points);
	m_platformVector.push_back(platform23);

	Platform platform24;
	vec = glm::vec2(3529, 534);
	platform24.points.push_back(vec);
	vec = glm::vec2(3801, 561);
	platform24.points.push_back(vec);
	vec = glm::vec2(3793, 674);
	platform24.points.push_back(vec);
	m_physics.createPlatform(platform24.points);
	m_platformVector.push_back(platform24);

	Platform platform25;
	vec = glm::vec2(3797, 744);
	platform25.points.push_back(vec);
	vec = glm::vec2(3801, 904);
	platform25.points.push_back(vec);
	vec = glm::vec2(3525, 911);
	platform25.points.push_back(vec);
	vec = glm::vec2(3536, 861);
	platform25.points.push_back(vec);
	m_physics.createPlatform(platform25.points);
	m_platformVector.push_back(platform25);

	Platform platform26;
	vec = glm::vec2(3797, 830);
	platform26.points.push_back(vec);
	vec = glm::vec2(3735, 892);
	platform26.points.push_back(vec);
	m_physics.createPlatform(platform26.points);
	m_platformVector.push_back(platform26);

	Platform platform27;
	vec = glm::vec2(3533, 1211);
	platform27.points.push_back(vec);
	vec = glm::vec2(3540, 1269);
	platform27.points.push_back(vec);
	vec = glm::vec2(3797, 1269);
	platform27.points.push_back(vec);
	vec = glm::vec2(3770, 1211);
	platform27.points.push_back(vec);
	m_physics.createPlatform(platform27.points);
	m_platformVector.push_back(platform27);

	Platform platform28;
	vec = glm::vec2(4213, 497);
	platform28.points.push_back(vec);
	vec = glm::vec2(4229, 574);
	platform28.points.push_back(vec);
	vec = glm::vec2(4559, 574);
	platform28.points.push_back(vec);
	vec = glm::vec2(4552, 489);
	platform28.points.push_back(vec);
	vec = glm::vec2(4221, 501);
	platform28.points.push_back(vec);
	m_physics.createPlatform(platform28.points);
	m_platformVector.push_back(platform28);

	Platform platform29;
	vec = glm::vec2(4257, 792);
	platform29.points.push_back(vec);
	vec = glm::vec2(4229, 882);
	platform29.points.push_back(vec);
	vec = glm::vec2(4560, 878);
	platform29.points.push_back(vec);
	vec = glm::vec2(4544, 777);
	platform29.points.push_back(vec);
	vec = glm::vec2(4248, 796);
	platform29.points.push_back(vec);
	m_physics.createPlatform(platform29.points);
	m_platformVector.push_back(platform29);

	Platform platform30;
	vec = glm::vec2(4557, 792);
	platform30.points.push_back(vec);
	vec = glm::vec2(5043, 886);
	platform30.points.push_back(vec);
	vec = glm::vec2(5428, 897);
	platform30.points.push_back(vec);
	vec = glm::vec2(5428, 948);
	platform30.points.push_back(vec);
	vec = glm::vec2(5074, 956);
	platform30.points.push_back(vec);
	vec = glm::vec2(5055, 897);
	platform30.points.push_back(vec);
	m_physics.createPlatform(platform30.points);
	m_platformVector.push_back(platform30);

	Platform platform31;
	vec = glm::vec2(4603, 489);
	platform31.points.push_back(vec);
	vec = glm::vec2(5019, 547);
	platform31.points.push_back(vec);
	vec = glm::vec2(5443, 539);
	platform31.points.push_back(vec);
	vec = glm::vec2(5432, 633);
	platform31.points.push_back(vec);
	vec = glm::vec2(5054, 629);
	platform31.points.push_back(vec);
	vec = glm::vec2(5043, 559);
	platform31.points.push_back(vec);
	m_physics.createPlatform(platform31.points);
	m_platformVector.push_back(platform31);

	Platform platform32;
	vec = glm::vec2(5442, 551);
	platform32.points.push_back(vec);
	vec = glm::vec2(5818, 516);
	platform32.points.push_back(vec);
	vec = glm::vec2(5853, 590);
	platform32.points.push_back(vec);
	vec = glm::vec2(6571, 586);
	platform32.points.push_back(vec);
	vec = glm::vec2(7118, 559);
	platform32.points.push_back(vec);
	vec = glm::vec2(7111, 520);
	platform32.points.push_back(vec);
	vec = glm::vec2(5816, 512);
	platform32.points.push_back(vec);
	m_physics.createPlatform(platform32.points);
	m_platformVector.push_back(platform32);

	Platform platform33;
	vec = glm::vec2(7084, 520);
	platform33.points.push_back(vec);
	vec = glm::vec2(7329, 298);
	platform33.points.push_back(vec);
	vec = glm::vec2(7671, 291);
	platform33.points.push_back(vec);
	m_physics.createPlatform(platform33.points);
	m_platformVector.push_back(platform33);

	Platform platform34;
	vec = glm::vec2(7411, 874);
	platform34.points.push_back(vec);
	vec = glm::vec2(7422, 967);
	platform34.points.push_back(vec);
	vec = glm::vec2(7656, 967);
	platform34.points.push_back(vec);
	vec = glm::vec2(7636, 870);
	platform34.points.push_back(vec);
	vec = glm::vec2(7368, 862);
	platform34.points.push_back(vec);
	m_physics.createPlatform(platform34.points);
	m_platformVector.push_back(platform34);

	Platform platform35;
	vec = glm::vec2(7407, 890);
	platform35.points.push_back(vec);
	vec = glm::vec2(7119, 866);
	platform35.points.push_back(vec);
	vec = glm::vec2(7119, 1255);
	platform35.points.push_back(vec);
	vec = glm::vec2(7014, 1232);
	platform35.points.push_back(vec);
	vec = glm::vec2(7033, 936);
	platform35.points.push_back(vec);
	vec = glm::vec2(6741, 917);
	platform35.points.push_back(vec);
	vec = glm::vec2(6714, 847);
	platform35.points.push_back(vec);
	vec = glm::vec2(7053, 847);
	platform35.points.push_back(vec);
	vec = glm::vec2(7033, 936);
	platform35.points.push_back(vec);
	m_physics.createPlatform(platform35.points);
	m_platformVector.push_back(platform35);

	Platform platform36;
	vec = glm::vec2(7033, 847);
	platform36.points.push_back(vec);
	vec = glm::vec2(7438, 878);
	platform36.points.push_back(vec);
	m_physics.createPlatform(platform36.points);
	m_platformVector.push_back(platform36);

	Platform platform37;
	vec = glm::vec2(6730, 862);
	platform37.points.push_back(vec);
	vec = glm::vec2(6247, 843);
	platform37.points.push_back(vec);
	vec = glm::vec2(6189, 928);
	platform37.points.push_back(vec);
	vec = glm::vec2(5870, 917);
	platform37.points.push_back(vec);
	vec = glm::vec2(5858, 851);
	platform37.points.push_back(vec);
	vec = glm::vec2(6228, 839);
	platform37.points.push_back(vec);
	m_physics.createPlatform(platform37.points);
	m_platformVector.push_back(platform37);

	Platform platform38;
	vec = glm::vec2(3073, 761);
	platform38.points.push_back(vec);
	vec = glm::vec2(3080, 808);
	platform38.points.push_back(vec);
	vec = glm::vec2(3115, 831);
	platform38.points.push_back(vec);
	vec = glm::vec2(3166, 851);
	platform38.points.push_back(vec);
	vec = glm::vec2(3220, 851);
	platform38.points.push_back(vec);
	vec = glm::vec2(3275, 835);
	platform38.points.push_back(vec);
	vec = glm::vec2(3318, 761);
	platform38.points.push_back(vec);
	m_physics.createPlatform(platform38.points);
	m_platformVector.push_back(platform38);

	Platform platform39;
	vec = glm::vec2(3123, 606);
	platform39.points.push_back(vec);
	vec = glm::vec2(3131, 648);
	platform39.points.push_back(vec);
	m_physics.createPlatform(platform39.points);
	m_platformVector.push_back(platform39);

	Platform platform40;
	vec = glm::vec2(3248, 602);
	platform40.points.push_back(vec);
	vec = glm::vec2(3255, 668);
	platform40.points.push_back(vec);
	m_physics.createPlatform(platform40.points);
	m_platformVector.push_back(platform40);

}
#endif