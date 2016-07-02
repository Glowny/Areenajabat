#pragma once

#include <glm\vec2.hpp>
#include <vector>
#include "Enumerations.h"

struct GladiatorData
{
	glm::vec2 position;
	glm::vec2 velocity;
	float rotation;
};

struct Platform
{
	std::vector<glm::vec2> points;
};

// Bullet data contains start point data of the bullet
// Bullet should be calculated from former gamestates.
// We could use separated bulletdata containing location
// to be send to clients, as server wont cheat.

// Data send by client to server.
struct BulletInputData
{
	// GameTime gameTime;
	BulletType bulletType;
	float rotation;
};

// Data send by server to clients
struct BulletOutputData
{
	// GameTime gameTime;
	unsigned playerId;
	BulletType bulletType;
	float rotation;
	// temporary, send only rotation!
	glm::vec2 velocity;

	glm::vec2 position;
};

MessageIdentifier getMessageID(unsigned char* data);
// returns beginning of data and saves size of packet on packetSize. If space for data is allocated, use preData.
unsigned char* createMovePacket(size_t &packetSize, glm::vec2 moveDirection, unsigned char* preData = NULL);
unsigned char* createUpdatePacket(size_t &packetSize, std::vector<GladiatorData> &gladiatorVector, unsigned char* preData = NULL);
unsigned char* createSetupPacket(size_t &packetSize, unsigned playerAmount, unsigned playerId, unsigned char* preData = NULL);
unsigned char* createPlatformPacket(size_t &packetSize, std::vector<Platform> &platformVector, unsigned char* preData = NULL);
unsigned char* createBulletInputPacket(size_t &packetSize, std::vector<BulletInputData> &bulletVector, unsigned char* preData = NULL);
unsigned char* createBulletOutputPacket(size_t &packetSize, std::vector<BulletOutputData> &bulletVector, unsigned char* preData = NULL);

void openMovePacket(unsigned char* data, glm::vec2 &moveDir);
void openUpdatePacket(unsigned char* data, std::vector<GladiatorData> &gladiatorVector);
void openSetupPacket(unsigned char* data, unsigned &playerAmount, unsigned &playerId);
void openPlatformPacket(unsigned char* data, std::vector<Platform> &platformVector);
void openBulletInputPacket(unsigned char* data, std::vector<BulletInputData> &bulletVector);
void openBulletOutputPacket(unsigned char* data, std::vector<BulletOutputData> &bulletVector);