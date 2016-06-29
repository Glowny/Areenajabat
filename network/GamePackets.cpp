#include "GamePackets.h"
#include "Serializer.h"

MessageIdentifier getMessageID(unsigned char* data)
{
	return getID(data);
}

unsigned char* createMovePacket(size_t &packetSize, glm::vec2 moveDirection, unsigned char* preData)
{
	packetSize = sizeof(MessageIdentifier) + sizeof(float) * 2;
	if (preData == NULL)
		preData = (unsigned char*)malloc(packetSize);
	
	unsigned char* dataPointer = preData;

	serializeSingle(dataPointer, ClientMove);
	serializeSingle(dataPointer, moveDirection.x);
	serializeSingle(dataPointer, moveDirection.y);
	
	return preData;
}

unsigned char* createUpdatePacket(size_t &packetSize, std::vector<GladiatorData> &gladiatorVector, unsigned char* preData)
{

	packetSize = sizeof(MessageIdentifier) + sizeof(float) * 5 * gladiatorVector.size();
	if (preData == NULL)
		preData = (unsigned char*)malloc(packetSize);
	
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


unsigned char* createSetupPacket(size_t &packetSize, unsigned playerAmount, unsigned playerId, unsigned char* preData)
{
	packetSize = sizeof(MessageIdentifier) + sizeof(unsigned) * 2;
	if (preData == NULL)
		preData = (unsigned char*)malloc(packetSize);

	unsigned char* dataPointer = preData;

	serializeSingle(dataPointer, Start);

	// send player amount and clients id for future use.
	serializeSingle(dataPointer, playerAmount);
	serializeSingle(dataPointer, playerId);
	return preData;
}

unsigned char* createPlatformPacket(size_t &packetSize, std::vector<Platform> &platformVector, unsigned char* preData)
{	
	// space for messageIdentifier and size of platformVector
	packetSize = sizeof(MessageIdentifier) + sizeof(size_t);
	for (unsigned i = 0; i < platformVector.size(); i++)
	{
		// space for the points of a platform and amount of points.
		packetSize += sizeof(float) * 2 * platformVector[i].points.size() + sizeof(size_t);
	}
	if (preData == NULL)
	{
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


void openMovePacket(unsigned char* data, glm::vec2 &moveDir)
{
	unsigned char* dataPointer = data + sizeof(MessageIdentifier);
	deSerializeSingle(dataPointer, moveDir.x);
	deSerializeSingle(dataPointer, moveDir.y);
	// TODO: Free data?
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

void openSetupPacket(unsigned char* data, unsigned &playerAmount, unsigned &playerId)
{
	unsigned char* dataPointer = data + sizeof(MessageIdentifier);
	deSerializeSingle(dataPointer, playerAmount);
	deSerializeSingle(dataPointer, playerId);
}

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
