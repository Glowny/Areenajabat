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

struct BulletHit
{
	bodyType hitType;
	glm::vec2 position;
	unsigned shooterPlayerId;
	unsigned targetPlayerId;
};

struct p_userData
{
	bodyType m_bodyType;
	void* m_object;
};

struct p_Platform
{
	b2ChainShape m_shape;
	b2BodyDef m_bodydef;
	b2Body* m_body;
	b2FixtureDef m_fixtureDef;
	p_userData* m_userData;
};


struct p_Gladiator
{
	unsigned m_id;
	b2Body* m_body;
	p_userData* m_userData;
};

struct p_Bullet
{
	unsigned m_shooterID;
	bool m_contact;
	bodyType m_contactBody;
	p_userData* m_contactUserData;
	b2Body* m_body;
	p_userData* m_userData;
	void startContact(bodyType contact, p_userData* contactUserData)
	{ 
		m_contact = true; m_contactBody = contact, m_contactUserData = contactUserData; 
	};

};
class ContactListener : public b2ContactListener
{
	void BeginContact(b2Contact* contact)
	{
		void* bodyUserData1 = contact->GetFixtureA()->GetBody()->GetUserData();
		void* bodyUserData2 = contact->GetFixtureB()->GetBody()->GetUserData();

		p_userData* userData1 = static_cast<p_userData*>(bodyUserData1);
		p_userData* userData2 = static_cast<p_userData*>(bodyUserData2);

		if (userData1->m_bodyType == B_Platform || userData1->m_bodyType == B_Gladiator)
		{
			if (userData2->m_bodyType == B_Bullet)
			{
				p_Bullet* bullet =  static_cast<p_Bullet*>(userData2->m_object);
				bullet->startContact(userData1->m_bodyType, userData1);
				p_Gladiator* glad =static_cast<p_Gladiator*>(userData1->m_object);
				printf("ads");
			}
		}

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
	unsigned addGladiator(glm::vec2 position, unsigned id);
	void AppleForceToGladiator(glm::vec2 direction, unsigned id);
	void ApplyImpulseToGladiator(glm::vec2 direction, unsigned id);
	glm::vec2 getGladiatorVelocity(unsigned id);
	glm::vec2 getGladiatorPosition(unsigned id);
	void setGladiatorPosition(unsigned id, glm::vec2 position);
	void removeGladiator(unsigned id);
	void addBullet(glm::vec2 position, glm::vec2 velocity, unsigned shooterID);
	void removeBullet();
	ContactListener m_ContactListener;
	std::vector<BulletHit> hitVector;
	std::vector<p_Bullet*> m_bulletVector;
private:
	b2World* m_b2DWorld;
	//TODO: Better way to communicate with server.
	std::vector<p_Gladiator> m_gladiatorVector;
	std::vector<p_Platform*> m_platformVector;
};
#endif