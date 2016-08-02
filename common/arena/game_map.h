#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <fstream>
#include <common/network_entity.h>
#include <assert.h>
#define MAXVERTEXAMOUNT 30

namespace arena
{

	enum ArenaPlatformType
	{
		SolidWall,
		LightWall,
		Ladder
	};
	struct ArenaPlatform
	{
		ArenaPlatformType type;
		std::vector<glm::vec2> vertices;
	};
	
	

	struct GameMap : public NetworkEntity
	{
		
		std::vector<ArenaPlatform> m_platformVector;
		glm::vec2 m_playerSpawnLocations[12]
		{
			glm::vec2(500,100),		glm::vec2(900,100),		glm::vec2(300, 100),	glm::vec2(400, 100),	glm::vec2(500, 100),
			glm::vec2(2000, 100),	glm::vec2(2100, 100),	glm::vec2(2200, 100),	glm::vec2(2300, 100),	glm::vec2(2400, 100),
			glm::vec2(5000, 666),	glm::vec2(5000, 666),
		};
	private:
		struct platformLoadObject
		{
			unsigned type;
			uint32_t size;
			std::vector<glm::vec2> vertex;
		};
	public:

		GameMap()
			: NetworkEntity(NetworkEntityType::Map)
		{

		}

		void loadMapFromFile(const char* const filename)
		{
			std::ifstream file;

			file.open(filename, std::ios::in | std::ios::binary);

			if (!file.is_open())
				printf("FILE NOT FOUND %s", filename);
			uint32_t sizes;
			file.read(reinterpret_cast<char*>(&sizes), sizeof(uint32_t));

			std::vector<platformLoadObject> objects;
			for (unsigned i = 0; i < sizes; i++)
			{
				platformLoadObject object;
				file.read(reinterpret_cast<char*>(&object.size), sizeof(uint32_t));
				file.read(reinterpret_cast<char*>(&object.type), sizeof(unsigned int));


				for (unsigned i = 0; i < object.size; i++)
				{
					float x;
					file.read(reinterpret_cast<char*>(&x), sizeof(float));
					object.vertex.push_back(glm::vec2(x, 0));
				}
				for (unsigned i = 0; i < object.size; i++)
				{
					float y;
					file.read(reinterpret_cast<char*>(&y), sizeof(float));
					object.vertex[i].y = y;
				}
				assert(object.vertex.size() < MAXVERTEXAMOUNT);
				objects.push_back(object);
			}
			file.close();

			for (unsigned i = 0; i < objects.size(); i++)
			{
				ArenaPlatform platform;
				platform.type = (ArenaPlatformType)objects[i].type;
				for (unsigned j = 0; j < objects[i].vertex.size(); j++)
				{
					platform.vertices.push_back(objects[i].vertex[j]);
				}
				m_platformVector.push_back(platform);
			}

		}
	};



}