#pragma once

#include <glm\vec2.hpp>
#include <vector>
#include "Enumerations.h"
#include <stdint.h>
// Move extra stuff away from here.
struct GladiatorData
{
	glm::vec2 position;
	glm::vec2 velocity;
	float rotation;
	unsigned hitPoints;
	bool alive;
	unsigned id;
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

struct PlayerScore
{
	int score;
	int tickets;
};
struct ScoreBoard
{
	std::vector<PlayerScore> PlayerScoreVector;
	unsigned flagHolder;
};

// Returns id of the packet.
MessageIdentifier getMessageID(unsigned char* data);

// Returns beginning of data and saves size of packet on packetSize. If space for data is allocated and size know, set the data on preData.
unsigned char* createUpdatePacket			(uint32_t &packetSize, std::vector<GladiatorData> &gladiatorVector,		unsigned char* preData = NULL);
unsigned char* createSetupPacket			(uint32_t &packetSize, unsigned playerAmount, unsigned playerId,		unsigned char* preData = NULL);
unsigned char* createPlatformPacket			(uint32_t &packetSize, std::vector<Platform> &platformVector,			unsigned char* preData = NULL);
unsigned char* createBulletCreationPacket	(uint32_t &packetSize, std::vector<BulletOutputData> &bulletVector,		unsigned char* preData = NULL);
unsigned char* createHitPacket				(uint32_t &packetSize, glm::vec2 hitPosition,							unsigned char* preData= NULL);
unsigned char* createPlayerDamagePacket		(uint32_t &packetSize, unsigned playerIndex, unsigned damageAmount,		unsigned char* preData = NULL);
unsigned char* createPlayerKillPacket		(uint32_t &packetSize, unsigned playerIndex,							unsigned char* preData = NULL);
unsigned char* createPlayerRespawnPacket	(uint32_t &packetSize, unsigned playerIndex,							unsigned char* preData = NULL);
unsigned char* createBulletUpdatePacket		(uint32_t &packetSize, std::vector<glm::vec2> &bulletPositions, 
											std::vector<glm::vec2> &bulletVelocities,								unsigned char* preData = NULL);
unsigned char* createScoreboardUpdatePacket	(uint32_t &packetSize, ScoreBoard &scoreBoard,							unsigned char* preData = NULL);

unsigned char* createMovePacket				(uint32_t &packetSize, glm::vec2 moveDirection,							unsigned char* preData = NULL);
unsigned char* createBulletRequestPacket	(uint32_t &packetSize, std::vector<BulletInputData> &bulletVector,		unsigned char* preData = NULL);
unsigned char* createSendPlayerAmountPacket		(uint32_t &packetSize, unsigned playerAmount,							unsigned char* preData = NULL);

// Sets data from the packet on parameters.
void openMovePacket				(unsigned char* data, glm::vec2 &moveDir);
void openBulletRequestPacket	(unsigned char* data, std::vector<BulletInputData> &bulletVector);


void openUpdatePacket			(unsigned char* data, std::vector<GladiatorData> &gladiatorVector);
void openSetupPacket			(unsigned char* data, unsigned &playerAmount, unsigned &playerId);
void openPlatformPacket			(unsigned char* data, std::vector<Platform> &platformVector);
void openBulletCreationPacket	(unsigned char* data, std::vector<BulletOutputData> &bulletVector);
void openHitPacket				(unsigned char* data, glm::vec2 &hitPosition);
void openPlayerDamagePacket		(unsigned char* data, unsigned &playerIndex, unsigned& damageAmount);
void openPlayerKillPacket		(unsigned char* data, unsigned &playerIndex);
void openPlayerRespawnPacket	(unsigned char* data, unsigned &playerIndex);
void openBulletUpdatePacket		(unsigned char* data, std::vector<glm::vec2> &bulletPositions, 
								std::vector<glm::vec2> &bulletVelocities);
void openScoreboardUpdatePacket	(unsigned char* data, ScoreBoard &scoreBoard);
void openPlayerAmountPacket	(unsigned char* data, unsigned &playerAmount);
