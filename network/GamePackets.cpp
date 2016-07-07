#include "GamePackets.h"
#include "Serializer.h"

MessageIdentifier getMessageID(unsigned char* data)
{
	return getID(data);
}

// Packets created by server and send to client.
unsigned char* createSetupPacket(uint32_t &packetSize, unsigned playerAmount, unsigned playerId, unsigned char* preData)
{
	// Send data about player amount and clients playerid.
	if (preData == NULL)
	{ 
		packetSize = sizeof(MessageIdentifier) + sizeof(unsigned) * 2;
		preData = (unsigned char*)malloc(packetSize);
	}
	unsigned char* dataPointer = preData;

	serializeSingle(dataPointer, Start);

	// send player amount and clients id for future use.
	serializeSingle(dataPointer, playerAmount);
	serializeSingle(dataPointer, playerId);
	return preData;
}
unsigned char* createPlatformPacket(uint32_t &packetSize, std::vector<Platform> &platformVector, unsigned char* preData)
{	
	// Send data about platforms. Send only on start of the game if no data exists on client.
	
	if (preData == NULL)
	{
		packetSize = sizeof(MessageIdentifier) + sizeof(uint32_t);
		for (unsigned i = 0; i < platformVector.size(); i++)
		{
			// space for the points of a platform and amount of points.
			packetSize += sizeof(float) * 2 * platformVector[i].points.size() + sizeof(uint32_t);
		}
		preData = (unsigned char*)malloc(packetSize);
	}
	unsigned char* dataPointer = preData;
	// Serialize id.
	serializeSingle(dataPointer, PlatformData);
	// Serialize amount of platforms.
	serializeSingle(dataPointer, uint32_t(platformVector.size()));
	for (unsigned platformIndex = 0; platformIndex < platformVector.size(); platformIndex++)
	{
		// Serialize amount of points in a platform
		serializeSingle(dataPointer, uint32_t(platformVector[platformIndex].points.size()));
		for (unsigned pointIndex = 0; pointIndex < platformVector[platformIndex].points.size(); pointIndex++)
		{ 
			// Serialize a single point.
			serializeSingle(dataPointer, platformVector[platformIndex].points[pointIndex].x);
			serializeSingle(dataPointer, platformVector[platformIndex].points[pointIndex].y);
		}
	}
	return preData;
}
unsigned char* createUpdatePacket(uint32_t &packetSize, std::vector<GladiatorData> &gladiatorVector, unsigned char* preData)
{
	// Send current gladiator positions and velocities.
	if (preData == NULL)
	{ 
		packetSize = sizeof(MessageIdentifier) + sizeof(float) * 5 * gladiatorVector.size();
		preData = (unsigned char*)malloc(packetSize);
	}
	unsigned char* dataPointer = preData;

	serializeSingle(dataPointer, Update);

	// Note, gladiator amount is static and send on setup-packet. If gladiator size changes, send size too.
	for (unsigned i = 0; i < gladiatorVector.size(); i++)
	{
		serializeSingle(dataPointer, gladiatorVector[i].position.x);
		serializeSingle(dataPointer, gladiatorVector[i].position.y);
		serializeSingle(dataPointer, gladiatorVector[i].velocity.x);
		serializeSingle(dataPointer, gladiatorVector[i].velocity.y);
		serializeSingle(dataPointer, gladiatorVector[i].rotation);
	}
	return preData;
}
unsigned char* createBulletUpdatePacket(uint32_t &packetSize, std::vector<glm::vec2> &bulletPositions, std::vector<glm::vec2> &bulletVelocities, unsigned char* preData)
{
	// For debugging, client simulates bullets itself.
	if (preData == NULL)
	{
		packetSize = sizeof(MessageIdentifier) + sizeof(uint32_t) + sizeof(float) * 4 * bulletPositions.size();
		preData = (unsigned char*)malloc(packetSize);
	}

	unsigned char* dataPointer = preData;

	serializeSingle(dataPointer, BulletUpdate);
	serializeSingle(dataPointer, uint32_t(bulletPositions.size()));

	for (unsigned i = 0; i < bulletPositions.size(); i++)
	{
		serializeSingle(dataPointer, bulletPositions[i].x);
		serializeSingle(dataPointer, bulletPositions[i].y);
		serializeSingle(dataPointer, bulletVelocities[i].x);
		serializeSingle(dataPointer, bulletVelocities[i].y);
	}
	return preData;
}
unsigned char* createBulletOutputPacket(uint32_t &packetSize, std::vector<BulletOutputData> &bulletVector, unsigned char* preData)
{
	// Sends data about bullet creation events.
	if (preData == NULL)
	{
		packetSize = sizeof(MessageIdentifier) +sizeof(uint32_t)+ (sizeof(BulletType) + sizeof(float) * 5)* bulletVector.size();
		preData = (unsigned char*)malloc(packetSize);
	}
	unsigned char* dataPointer = preData;

	serializeSingle(dataPointer, CreateBullet);
	serializeSingle(dataPointer, uint32_t(bulletVector.size()));

	for (unsigned i = 0; i < bulletVector.size(); i++)
	{
		serializeSingle(dataPointer, bulletVector[i].bulletType);
		serializeSingle(dataPointer, bulletVector[i].rotation);
		serializeSingle(dataPointer, bulletVector[i].position.x);
		serializeSingle(dataPointer, bulletVector[i].position.y);
		serializeSingle(dataPointer, bulletVector[i].velocity.x);
		serializeSingle(dataPointer, bulletVector[i].velocity.y);
	}
	return preData;
}
unsigned char* createHitPacket(uint32_t &packetSize, glm::vec2 hitPosition, unsigned char* preData)
{
	// Send data about bullet hits on objects.
	if (preData == NULL)
	{
		packetSize = sizeof(MessageIdentifier) + sizeof(float) * 2;
		preData = (unsigned char*)malloc(packetSize);
	}
	unsigned char* dataPointer = preData;

	serializeSingle(dataPointer, Hit);

	serializeSingle(dataPointer, hitPosition.x);
	serializeSingle(dataPointer, hitPosition.y);
	
	return preData;
}
unsigned char* createPlayerDamagePacket(uint32_t &packetSize, unsigned playerIndex, unsigned damageAmount, unsigned char* preData)
{
	// Player takes damage.
	if (preData == NULL)
	{
		packetSize = sizeof(MessageIdentifier) + sizeof(unsigned) * 2;
		preData = (unsigned char*)malloc(packetSize);
	}
	unsigned char* dataPointer = preData;

	serializeSingle(dataPointer, PlayerDamage);

	serializeSingle(dataPointer, playerIndex);
	serializeSingle(dataPointer, damageAmount);

	return preData;
}
unsigned char* createPlayerKillPacket(uint32_t &packetSize, unsigned playerIndex, unsigned char* preData)
{
	// Player is killed.
	if (preData == NULL)
	{
		packetSize = sizeof(MessageIdentifier) + sizeof(unsigned);
		preData = (unsigned char*)malloc(packetSize);
	}
	unsigned char* dataPointer = preData;

	serializeSingle(dataPointer, PlayerKill);

	serializeSingle(dataPointer, playerIndex);

	return preData;
}
unsigned char* createPlayerRespawnPacket(uint32_t &packetSize, unsigned playerIndex, unsigned char* preData)
{
	// Player has been respawned.
	if (preData == NULL)
	{
		packetSize = sizeof(MessageIdentifier) + sizeof(unsigned);
		preData = (unsigned char*)malloc(packetSize);
	}
	unsigned char* dataPointer = preData;

	serializeSingle(dataPointer, PlayerRespawn);

	serializeSingle(dataPointer, playerIndex);

	return preData;
}
unsigned char* createScoreboardUpdatePacket(uint32_t &packetSize, ScoreBoard &scoreBoard, unsigned char* preData)
{
	if (preData == NULL)
	{
		packetSize = sizeof(MessageIdentifier) +sizeof(unsigned)+ sizeof(int) * 2 * scoreBoard.PlayerScoreVector.size();
		preData = (unsigned char*)malloc(packetSize);
	}
	unsigned char* dataPointer = preData;
	serializeSingle(dataPointer, ScoreBoardUpdate);
	serializeSingle(dataPointer, scoreBoard.flagHolder);
	// No need to serialize size; Player amount wont change during game.
	for(unsigned i = 0; i < scoreBoard.PlayerScoreVector.size(); i++)
	{ 
		serializeSingle(dataPointer, scoreBoard.PlayerScoreVector[i].score);
		serializeSingle(dataPointer, scoreBoard.PlayerScoreVector[i].tickets);
	}

	return preData;
}

// Packets created by client and send to server.
unsigned char* createMovePacket(uint32_t &packetSize, glm::vec2 moveDirection, unsigned char* preData)
{
	// Player wants to move in direction.
	if (preData == NULL)
	{ 
		packetSize = sizeof(MessageIdentifier) + sizeof(float) * 2;
		preData = (unsigned char*)malloc(packetSize);
	}
	unsigned char* dataPointer = preData;

	serializeSingle(dataPointer, ClientMove);
	serializeSingle(dataPointer, moveDirection.x);
	serializeSingle(dataPointer, moveDirection.y);
	
	return preData;
}
unsigned char* createBulletInputPacket(uint32_t &packetSize, std::vector<BulletInputData> &bulletVector, unsigned char* preData)
{
	// Player wants to create a bullet.
	if (preData == NULL)
	{
		packetSize = sizeof(MessageIdentifier) +sizeof(uint32_t)+ (sizeof(float) + sizeof(unsigned) + sizeof(BulletType)) * bulletVector.size();
		preData = (unsigned char*)malloc(packetSize);
	}
	unsigned char* dataPointer = preData;

	serializeSingle(dataPointer, ClientShoot);
	serializeSingle(dataPointer, uint32_t(bulletVector.size()));
	for (unsigned i = 0; i < bulletVector.size(); i++)
	{
		serializeSingle(dataPointer, bulletVector[i].bulletType);
		serializeSingle(dataPointer, bulletVector[i].rotation);
	}
	return preData;
}

// Server side packet opens.
void openMovePacket(unsigned char* data, glm::vec2 &moveDir)
{
	unsigned char* dataPointer = data + sizeof(MessageIdentifier);
	deSerializeSingle(dataPointer, moveDir.x);
	deSerializeSingle(dataPointer, moveDir.y);

}
void openBulletInputPacket(unsigned char* data, std::vector<BulletInputData> &bulletVector)
{
	unsigned char* dataPointer = data + sizeof(MessageIdentifier);
	uint32_t bulletAmount;
	deSerializeSingle(dataPointer, bulletAmount);
	for (unsigned i = 0; i < bulletAmount; i++)
	{
		BulletInputData bulletData;
		deSerializeSingle(dataPointer, bulletData.bulletType);
		deSerializeSingle(dataPointer, bulletData.rotation);
		float rotation = bulletData.rotation;
		bulletVector.push_back(bulletData);
	}
}

// Client side packet opens.
void openSetupPacket(unsigned char* data, unsigned &playerAmount, unsigned &playerId)
{
	unsigned char* dataPointer = data + sizeof(MessageIdentifier);
	deSerializeSingle(dataPointer, playerAmount);
	deSerializeSingle(dataPointer, playerId);
}
void openPlatformPacket(unsigned char* data, std::vector<Platform> &platformVector)
{
	unsigned char* dataPointer = data + sizeof(MessageIdentifier);
	uint32_t platformAmount;
	// Get amount of platforms.
	deSerializeSingle(dataPointer, platformAmount);

	uint32_t pointAmount;

	for (unsigned platformIndex = 0; platformIndex < platformAmount; platformIndex++)
	{
		// Get amount of points in platform
		deSerializeSingle(dataPointer, pointAmount);
		Platform platform;
		glm::vec2 point;

		for (unsigned pointIndex = 0; pointIndex < pointAmount; pointIndex++)
		{
			// Get single point
			deSerializeSingle(dataPointer, point.x);
			deSerializeSingle(dataPointer, point.y);
			platform.points.push_back(point);
		}

		platformVector.push_back(platform);
	}
	
}
void openUpdatePacket(unsigned char* data, std::vector<GladiatorData> &gladiatorVector)
{
	// If nothing is done with the data after this, "data" could be used instead of "dataPointer".
	unsigned char* dataPointer = data + sizeof(MessageIdentifier);
	// Get amount of gladiators from gladiatorVector size. If gladiator amount is not pre-set, data should be read from message.
	for (unsigned i = 0; i < gladiatorVector.size(); i++)
	{
		deSerializeSingle(dataPointer, gladiatorVector[i].position.x);
		deSerializeSingle(dataPointer, gladiatorVector[i].position.y);
		deSerializeSingle(dataPointer, gladiatorVector[i].velocity.x);
		deSerializeSingle(dataPointer, gladiatorVector[i].velocity.y);
		deSerializeSingle(dataPointer, gladiatorVector[i].rotation);
	}
}
void openBulletUpdatePacket(unsigned char* data, std::vector<glm::vec2> &bulletPositions, std::vector<glm::vec2> &bulletVelocities)
{
	unsigned char* dataPointer = data + sizeof(MessageIdentifier);
	uint32_t size;
	deSerializeSingle(dataPointer, size);
	for (unsigned i = 0; i < size; i++)
	{
		glm::vec2 bullet;
		bulletPositions.push_back(bullet);
		bulletVelocities.push_back(bullet);
		deSerializeSingle(dataPointer, bulletPositions[i].x);
		deSerializeSingle(dataPointer, bulletPositions[i].y);
		
		deSerializeSingle(dataPointer, bulletVelocities[i].x);
		deSerializeSingle(dataPointer, bulletVelocities[i].y);
	}

}
void openBulletOutputPacket(unsigned char* data, std::vector<BulletOutputData> &bulletVector)
{
	unsigned char* dataPointer = data + sizeof(MessageIdentifier);
	uint32_t bulletAmount;
	deSerializeSingle(dataPointer, bulletAmount);

	for (unsigned i = 0; i < bulletAmount; i++)
	{
		bulletVector.push_back(BulletOutputData());
		deSerializeSingle(dataPointer, bulletVector[i].bulletType);
		deSerializeSingle(dataPointer, bulletVector[i].rotation);
		deSerializeSingle(dataPointer, bulletVector[i].position.x);
		deSerializeSingle(dataPointer, bulletVector[i].position.y);
		deSerializeSingle(dataPointer, bulletVector[i].velocity.x);
		deSerializeSingle(dataPointer, bulletVector[i].velocity.y);
	}
}
void openHitPacket(unsigned char* data, glm::vec2 &hitPosition)
{
	unsigned char* dataPointer = data + sizeof(MessageIdentifier);
	deSerializeSingle(dataPointer, hitPosition.x);
	deSerializeSingle(dataPointer, hitPosition.y);
}
void openPlayerDamagePacket(unsigned char* data, unsigned &playerIndex, unsigned &damageAmount)
{
	unsigned char* dataPointer = data + sizeof(MessageIdentifier);
	deSerializeSingle(dataPointer, playerIndex);
	deSerializeSingle(dataPointer, damageAmount);
}
void openPlayerKillPacket(unsigned char* data, unsigned &playerIndex)
{
	unsigned char* dataPointer = data + sizeof(MessageIdentifier);
	deSerializeSingle(dataPointer, playerIndex);
}
void openPlayerRespawnPacket(unsigned char* data, unsigned &playerIndex)
{
	unsigned char* dataPointer = data + sizeof(MessageIdentifier);
	deSerializeSingle(dataPointer, playerIndex);
}
void openScoreboardUpdatePacket(unsigned char* data, ScoreBoard &scoreBoard)
{
	unsigned char* dataPointer = data+sizeof(MessageIdentifier);
	deSerializeSingle(dataPointer, scoreBoard.flagHolder);
	
	// Scoreboard has to be set before use!
	for (unsigned i = 0; i < scoreBoard.PlayerScoreVector.size(); i++)
	{
		deSerializeSingle(dataPointer, scoreBoard.PlayerScoreVector[i].score);
		deSerializeSingle(dataPointer, scoreBoard.PlayerScoreVector[i].tickets);
	}
}