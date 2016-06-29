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

MessageIdentifier getMessageID(unsigned char* data);
// returns beginning of data and saves size of packet on packetSize. If space for data is allocated, use preData.
unsigned char* createMovePacket(size_t &packetSize, glm::vec2 moveDirection, unsigned char* preData = NULL);
unsigned char* createUpdatePacket(size_t &packetSize, std::vector<GladiatorData> &gladiatorVector, unsigned char* preData = NULL);
unsigned char* createSetupPacket(size_t &packetSize, unsigned playerAmount, unsigned playerId, unsigned char* preData = NULL);
unsigned char* createPlatformPacket(size_t &packetSize, std::vector<Platform> &platformVector, unsigned char* preData = NULL);

void openMovePacket(unsigned char* data, glm::vec2 &moveDir);
void openUpdatePacket(unsigned char* data, std::vector<GladiatorData> &gladiatorVector);
void openSetupPacket(unsigned char* data, unsigned &playerAmount, unsigned &playerId);
void openPlatformPacket(unsigned char* data, std::vector<Platform> &platformVector);
