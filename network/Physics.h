#pragma once
#if defined(ARENA_SERVER)
#include <Box2D\Box2D.h>
#include <stdarg.h>
#include <vector>
#include <glm\glm.hpp>

// TODO: platform has extra stuff that could be removed.

enum bodyType
{
	B_Platform,
	B_Gladiator,
	B_Bullet,
	B_NONE
};

struct p_Platform
{
	b2ChainShape m_shape;
	b2BodyDef m_bodydef;
	b2Body* m_body;
	b2FixtureDef m_fixtureDef;
};


struct p_Gladiator
{
	unsigned m_id;
	b2Body* m_body;
};

struct p_Bullet
{
	bool m_contact;
	bodyType m_contactBody;
	b2Body* m_body;
	void startContact(bodyType contact) 
	{ 
		m_contact = true; m_contactBody = contact; 
	};

};
class ContactListener : public b2ContactListener
{
	void BeginContact(b2Contact* contact)
	{
		void* bodyUserData = contact->GetFixtureA()->GetBody()->GetUserData();
		bodyType type = *((bodyType*)bodyUserData);
		if (type == B_Platform)
		{ 
			
			void* bodyUserData2 = contact->GetFixtureB()->GetBody()->GetUserData();
			
			bodyType type2 = *((bodyType*)bodyUserData2);
			if (type2 == B_Bullet)
			{
				p_Bullet* bullet = static_cast<p_Bullet*>(bodyUserData2);
				bullet->startContact(B_Platform);
			}
			
		}
	}
};
class Physics
{
	bodyType types[3]
	{
		B_Platform,
		B_Gladiator,
		B_Bullet
	};
	// add movement functions
	// add functions for getting position and velocity data
public:
	Physics();
	~Physics();

	void update();

	void createPlatform(std::vector<glm::vec2> platform);
	unsigned addGladiator(glm::vec2 position);
	void moveGladiator(glm::vec2 direction, unsigned id);
	glm::vec2 getGladiatorVelocity(unsigned id);
	glm::vec2 getGladiatorPosition(unsigned id);
	void removeGladiator(unsigned id);
	void addBullet(glm::vec2 position, glm::vec2 velocity);
	void removeBullet();
	ContactListener m_ContactListener;
private:
	b2World* m_b2DWorld;
	std::vector<p_Gladiator> m_gladiatorVector;
	std::vector<p_Platform*> m_platformVector;
	std::vector<p_Bullet*> m_bulletVector;
};
#endif