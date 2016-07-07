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
	p_userData* m_myUserData;
	void startContact(bodyType contact, p_userData* contactUserData)
	{ 
		m_contact = true; m_contactBody = contact, m_contactUserData = contactUserData; 
	};

};
class ContactListener : public b2ContactListener
{
	void BeginContact(b2Contact* contact)
	{
		void* targetBodyUserData = contact->GetFixtureA()->GetBody()->GetUserData();
		void* bulletBodyUserData = contact->GetFixtureB()->GetBody()->GetUserData();

		p_userData* targetUserData = static_cast<p_userData*>(targetBodyUserData);
		p_userData* bulletUserData = static_cast<p_userData*>(bulletBodyUserData);

		if (targetUserData->m_bodyType == B_Platform || targetUserData->m_bodyType == B_Gladiator)
		{
			if (bulletUserData->m_bodyType == B_Bullet)
			{
				p_Bullet* bullet =  static_cast<p_Bullet*>(bulletUserData->m_object);
				bullet->startContact(targetUserData->m_bodyType, targetUserData);
				p_Gladiator* glad =static_cast<p_Gladiator*>(targetUserData->m_object);
				printf("Gladiator id: %d", &glad->m_id);
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
	std::vector<p_Gladiator*> m_gladiatorVector;
	std::vector<p_Platform*> m_platformVector;
};
#endif