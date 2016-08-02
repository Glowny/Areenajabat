#pragma once
#include <bx/macros.h>

BX_PRAGMA_DIAGNOSTIC_PUSH()
BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4265) // class has virtual functions, but destructor is not virtual	
#include <functional>
BX_PRAGMA_DIAGNOSTIC_POP()

#include <Box2D/Box2D.h>
#include <stdarg.h>
#include <vector>
#include <glm/glm.hpp>
#include <typeinfo>
#include "../network_entity.h"

// TODO: platform has extra stuff that could be removed.

enum entityCategory
{
	c_Platform =				0x0001,
	c_LightPlatform=			0x0002,
	c_Ladder =					0x0004,
	c_GladiatorNoCollide=		0x0008,
	c_Gladiator=				0x0010,
	c_Bullet=					0x0020,

};
enum entityIndexes
{
	ci_Platform = 0,
	ci_LightPlatform = 1,
	ci_Ladder = 2,
	ci_GladiatorNoCollide = 3,
	ci_Gladiator = 4,
	ci_Bullet = 5,

};

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
	uint32_t m_id;
	b2Body* m_body;
	p_userData* m_userData;
	glm::vec2* gamePosition;

};

struct p_Bullet
{
	unsigned m_shooterID;
	uint8_t bulletId;
	bool m_contact;
	bodyType m_contactBody;
	p_userData* m_contactUserData;
	b2Body* m_body;
	p_userData* m_myUserData;
	glm::vec2* gamePosition;
	glm::vec2 hitPosition;

	void cleanUp()
	{
		delete m_myUserData;
		m_body->GetWorld()->DestroyBody(m_body);
	}
	void startContact(p_userData* contactUserData)
	{ 
		m_contact = true; m_contactUserData = contactUserData; 
		hitPosition.x = m_body->GetPosition().x;
		hitPosition.y = m_body->GetPosition().y;
	};

};

struct BulletCollisionEntry final 
{
	p_Bullet		m_bullet;

	p_Gladiator		m_shooter;
	p_Gladiator		m_target;
};

class ContactListener : public b2ContactListener
{
public:
	std::vector<BulletCollisionEntry> m_bulletCollisionEntries;

	std::vector<p_Gladiator*>* m_gladiators { nullptr };

	ContactListener(std::vector<p_Gladiator*>* gladiators) : b2ContactListener(),
															 m_gladiators(gladiators)
	{
	}

	~ContactListener() = default;
private:
	p_Gladiator* findGladiator(const uint32 id)
	{
		if (m_gladiators == nullptr) return nullptr;
		
		for (p_Gladiator* g : *m_gladiators) if (g->m_id == id) return g;

		return nullptr;
	}

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
				bullet->startContact( targetUserData);

				BulletCollisionEntry entry;
				entry.m_bullet	= *bullet;
				entry.m_shooter = *findGladiator(bullet->m_shooterID);
				entry.m_target	= *static_cast<p_Gladiator*>(targetUserData->m_object);
				
				m_bulletCollisionEntries.push_back(entry);
			}
		}

	}
};


//class ArenaContactListener : public ContactListener {
//	// Called when two fixtures begin to touch
//	virtual void BeginContact(b2Contact* contact) final override;
//
//	// Called when two fixtures cease to touch
//	virtual void EndContact(b2Contact* contact) final override;
//};

using CollisionCallback = std::function<void(arena::NetworkEntity* const, arena::NetworkEntity* const)>;

class Physics
{
public:
	b2Filter b2Filters[6];
	Physics();
	~Physics();

	float32 updateTimer;
	void reset();
	void update(float32 timeStep = 1.0f / 60.0f);
	void createPlatform(std::vector<glm::vec2> platform, unsigned type);
	void setGladiatorCollideLightPlatforms(unsigned gladiatorID, bool collide);
	unsigned addGladiator(glm::vec2* position);
	void applyForceToGladiator(glm::vec2 direction, unsigned id);
	void applyImpulseToGladiator(glm::vec2 direction, unsigned id);
	glm::vec2 getGladiatorVelocity(unsigned id);
	glm::vec2 getGladiatorPosition(unsigned id);

	//void addCollisionCallback(CollisionCallback callback);
	//void removeCollisionCallback(CollisionCallback callback);

	void setGladiatorPosition(unsigned id, glm::vec2 position);
	void removeGladiator(unsigned id);
	uint8_t addBullet(glm::vec2* position, glm::vec2 velocity, unsigned shooterID);
	void removeBullet(uint8_t id);
	ContactListener m_ContactListener;

	std::vector<BulletHit> hitVector;
	std::vector<p_Bullet*> m_bulletVector;

	uint8_t getFreeBulletId();
private:
	
	//ArenaContactListener m_listener;

	std::vector<CollisionCallback> m_callbacks;
	int16 gladiatorIdToGroupId(uint32_t playerId);
	bool isIdFree[256];
	void nextUint8_t(uint8_t& current);
	uint8_t currentFreeId;
	b2World* m_b2DWorld;
	//TODO: Better way to communicate with server.
	std::vector<p_Gladiator*> m_gladiatorVector;
	std::vector<p_Platform*> m_platformVector;
};
