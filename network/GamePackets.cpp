#include "GamePackets.h"
#include "Serializer.h"

MessageIdentifier getMessageID(unsigned char* data)
{
	return getID(data);
}

// Client -> Server
unsigned char* createMovePacket(size_t &packetSize, glm::vec2 moveDirection, unsigned char* preData)
{
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
// Server -> Client
unsigned char* createUpdatePacket(size_t &packetSize, std::vector<GladiatorData> &gladiatorVector, unsigned char* preData)
{

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

// Server -> Client
unsigned char* createSetupPacket(size_t &packetSize, unsigned playerAmount, unsigned playerId, unsigned char* preData)
{
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

// Server -> Client
unsigned char* createPlatformPacket(size_t &packetSize, std::vector<Platform> &platformVector, unsigned char* preData)
{	
	// space for messageIdentifier and size of platformVector
	
	if (preData == NULL)
	{
		packetSize = sizeof(MessageIdentifier) + sizeof(size_t);
		for (unsigned i = 0; i < platformVector.size(); i++)
		{
			// space for the points of a platform and amount of points.
			packetSize += sizeof(float) * 2 * platformVector[i].points.size() + sizeof(size_t);
		}
		preData = (unsigned char*)malloc(packetSize);
	}
	unsigned char* dataPointer = preData;
	// Serialize id.
	serializeSingle(dataPointer, PlatformData);
	// Serialize amount of platforms.
	serializeSingle(dataPointer, platformVector.size());
	for (unsigned platformIndex = 0; platformIndex < platformVector.size(); platformIndex++)
	{
		// Serialize amount of points in a platform
		serializeSingle(dataPointer, platformVector[platformIndex].points.size());
		for (unsigned pointIndex = 0; pointIndex < platformVector[platformIndex].points.size(); pointIndex++)
		{ 
			// Serialize a single point.
			serializeSingle(dataPointer, platformVector[platformIndex].points[pointIndex].x);
			serializeSingle(dataPointer, platformVector[platformIndex].points[pointIndex].y);
		}
	}
	return preData;
}

// Client -> Server
unsigned char* createBulletInputPacket(size_t &packetSize, std::vector<BulletInputData> &bulletVector, unsigned char* preData)
{
	if (preData == NULL)
	{
		packetSize = sizeof(MessageIdentifier) +sizeof(size_t)+ (sizeof(float) + sizeof(unsigned) + sizeof(BulletType)) * bulletVector.size();
		preData = (unsigned char*)malloc(packetSize);
	}
	unsigned char* dataPointer = preData;

	serializeSingle(dataPointer, ClientShoot);
	serializeSingle(dataPointer, bulletVector.size());
	for (unsigned i = 0; i < bulletVector.size(); i++)
	{
		serializeSingle(dataPointer, bulletVector[i].bulletType);
		serializeSingle(dataPointer, bulletVector[i].rotation);
	}
	return preData;
}

// Server -> Client
unsigned char* createBulletOutputPacket(size_t &packetSize, std::vector<BulletOutputData> &bulletVector, unsigned char* preData)
{

	if (preData == NULL)
	{
		packetSize = sizeof(MessageIdentifier) +sizeof(size_t)+ (sizeof(BulletType) + sizeof(float) * 5)* bulletVector.size();
		preData = (unsigned char*)malloc(packetSize);
	}
	unsigned char* dataPointer = preData;

	serializeSingle(dataPointer, CreateBullet);
	serializeSingle(dataPointer, bulletVector.size());

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

// Server -> Client
unsigned char* createHitPacket(size_t &packetSize, glm::vec2 hitPosition, unsigned char* preData)
{
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
// Server --> Client, temporary, for debugging
unsigned char* createBulletUpdatePacket(size_t &packetSize, std::vector<glm::vec2> &bulletPositions, unsigned char* preData)
{
	if (preData == NULL)
	{
		packetSize = sizeof(MessageIdentifier) + sizeof(size_t) + sizeof(float) * 2 * bulletPositions.size();
		preData = (unsigned char*)malloc(packetSize);
	}

	unsigned char* dataPointer = preData;

	serializeSingle(dataPointer, BulletUpdate);
	serializeSingle(dataPointer, bulletPositions.size());

	for (unsigned i = 0; i < bulletPositions.size(); i++)
	{
		serializeSingle(dataPointer, bulletPositions[i].x);
		serializeSingle(dataPointer, bulletPositions[i].y);
	}
	return preData;
}

// Server
void openMovePacket(unsigned char* data, glm::vec2 &moveDir)
{
	unsigned char* dataPointer = data + sizeof(MessageIdentifier);
	deSerializeSingle(dataPointer, moveDir.x);
	deSerializeSingle(dataPointer, moveDir.y);

}

// Client
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

// Client
void openSetupPacket(unsigned char* data, unsigned &playerAmount, unsigned &playerId)
{
	unsigned char* dataPointer = data + sizeof(MessageIdentifier);
	deSerializeSingle(dataPointer, playerAmount);
	deSerializeSingle(dataPointer, playerId);
}

// Client
void openPlatformPacket(unsigned char* data, std::vector<Platform> &platformVector)
{
	unsigned char* dataPointer = data + sizeof(MessageIdentifier);
	size_t platformAmount;
	// Get amount of platforms.
	deSerializeSingle(dataPointer, platformAmount);

	size_t pointAmount;

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

// Server
void openBulletInputPacket(unsigned char* data, std::vector<BulletInputData> &bulletVector)
{
	unsigned char* dataPointer = data + sizeof(MessageIdentifier);
	size_t bulletAmount;
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

// Client
void openBulletOutputPacket(unsigned char* data, std::vector<BulletOutputData> &bulletVector)
{
	unsigned char* dataPointer = data + sizeof(MessageIdentifier);
	size_t bulletAmount;
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

// Client
void openHitPacket(unsigned char* data, glm::vec2 &hitPosition)
{
	unsigned char* dataPointer = data + sizeof(MessageIdentifier);
	deSerializeSingle(dataPointer, hitPosition.x);
	deSerializeSingle(dataPointer, hitPosition.y);
}

void openBulletUpdatePacket(unsigned char* data, std::vector<glm::vec2> &bulletPositions)
{
	unsigned char* dataPointer = data + sizeof(MessageIdentifier);
	size_t size;
	deSerializeSingle(dataPointer, size);
	for (unsigned i = 0; i < size; i++)
	{
		glm::vec2 bullet;
		bulletPositions.push_back(bullet);
		
		deSerializeSingle(dataPointer, bulletPositions[i].x);
		deSerializeSingle(dataPointer, bulletPositions[i].y);
		
	}

}