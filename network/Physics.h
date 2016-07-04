#pragma once
#if defined(ARENA_SERVER)
#include <Box2D\Box2D.h>
#include <stdarg.h>
#include <vector>
#include <glm\glm.hpp>
#include <typeinfo>
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
	bodyType m_bodyType;
	b2ChainShape m_shape;
	b2BodyDef m_bodydef;
	b2Body* m_body;
	b2FixtureDef m_fixtureDef;
};


struct p_Gladiator
{
	bodyType m_bodyType;
	unsigned m_id;
	b2Body* m_body;
};

struct p_Bullet
{
	bodyType m_bodyType;
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
		void* bodyUserData1 = contact->GetFixtureA()->GetBody()->GetUserData();
		void* bodyUserData2 = contact->GetFixtureB()->GetBody()->GetUserData();
	
	}
};
class Physics
{

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