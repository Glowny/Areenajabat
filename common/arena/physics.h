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
#include "weapons.h"


#define PHYSICS_TIMESTEP 0.016f
// TODO: platform has extra stuff that could be removed.

enum entityCategory
{
	c_Platform =				0x0001,
	c_LightPlatform=			0x0002,
	c_Ladder =					0x0004,
	c_GladiatorNoCollide=		0x0008,
	c_Gladiator=				0x0010,
	c_GladiatorJumpCollider=	0x0020,
	c_JumpSensor =				0x0040,
	c_Bullet =					0x0080,
	c_BulletSensor =			0x0100,
	c_Grenade=					0x0200,
};
enum entityIndexes
{
	ci_Platform					= 0,
	ci_LightPlatform			= 1,
	ci_Ladder					= 2,
	ci_GladiatorNoCollide		= 3,
	ci_Gladiator				= 4,
	ci_GladiatorJumpCollider	= 5,
	ci_JumpSensor				= 6,
	ci_Bullet					= 7,
	ci_BulletSensor				= 8,
	ci_Grenade					= 9,
	ci_Count,

};

enum bodyType
{
	B_Platform,
	B_Grenade,
	B_Explosion,
	B_Gladiator,
	B_Bullet,
	B_LadderLeft,
	B_LadderRight,
	B_NONE
};

struct p_entity
{
	bodyType m_type;
	uint8_t m_id;
	virtual void cleanUp() {};
	virtual ~p_entity() {};
	b2Body* m_body = nullptr;
};

struct BulletHit : public p_entity
{
	bodyType hitType;
	glm::vec2 position;
	unsigned shooterPlayerId;
	unsigned targetPlayerId;
};


struct p_userData
{
	bodyType m_bodyType;
	p_entity* m_object;
	
};


struct p_Platform :public p_entity
{
	b2ChainShape m_shape;
	b2BodyDef m_bodydef;
	b2FixtureDef m_fixtureDef;
	p_userData* m_userData;
	void cleanUp()
	{
		delete m_userData;
		m_body->GetWorld()->DestroyBody(m_body);
	}
};


struct p_Gladiator :public p_entity
{
	p_userData* m_userData;
	b2Body* m_sensorBody;
	glm::vec2* m_gamePosition;
	glm::vec2* m_gamevelocity;

	void cleanUp()
	{
		delete m_userData;
		m_body->GetWorld()->DestroyBody(m_body);
	}

};


struct p_Bullet :public p_entity
{
	unsigned m_shooterID;
	bool m_contact;
	bodyType m_contactBody;
	arena::BulletType m_bulletType;
	p_userData* m_contactUserData;
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
	p_entity*		m_target;
};

class ContactListener : public b2ContactListener
{
public:
	std::vector<BulletCollisionEntry> m_bulletCollisionEntries;
	
	//std::vector<p_Platform*>* m_platforms{ nullptr };
	std::vector<p_entity*>* m_entities{ nullptr };

	ContactListener(std::vector<p_entity*>* entities) : b2ContactListener(),
		m_entities(entities)
	{
	}

	~ContactListener() = default;
private:
	p_entity* findEntity(const uint8_t id)
	{
		if (m_entities == nullptr) return nullptr;
		
		for (p_entity* g : *m_entities) if (g->m_id == id) return g;

		return nullptr;
	}

	void BeginContact(b2Contact* contact)
	{
		void* targetBodyUserData = contact->GetFixtureA()->GetBody()->GetUserData();
		void* bulletBodyUserData = contact->GetFixtureB()->GetBody()->GetUserData();

		p_userData* targetUserData = static_cast<p_userData*>(targetBodyUserData);
		p_userData* bulletUserData = static_cast<p_userData*>(bulletBodyUserData);

		if (bulletUserData->m_bodyType == B_Bullet || bulletUserData->m_bodyType == B_Explosion)
		{
			switch (targetUserData->m_bodyType)
			{
				case B_Platform:
				{
					if (bulletUserData->m_bodyType == B_Explosion)
						return;
					p_Bullet* bullet = static_cast<p_Bullet*>(bulletUserData->m_object);
					bullet->startContact(targetUserData);

					BulletCollisionEntry entry;
					entry.m_bullet = *bullet;

					p_Platform* entryPlatform = new p_Platform;
					*entryPlatform = *static_cast<p_Platform*>(targetUserData->m_object);
					entry.m_target = entryPlatform;

					m_bulletCollisionEntries.push_back(entry);
				}
				break;

				case B_Gladiator:
				{
					p_Bullet* bullet = static_cast<p_Bullet*>(bulletUserData->m_object);
					bullet->startContact(targetUserData);

					BulletCollisionEntry entry;
					entry.m_bullet = *bullet;
					entry.m_bullet.m_bulletType;
					entry.m_shooter = *static_cast<p_Gladiator*>(findEntity(bullet->m_shooterID));
					 
					p_Gladiator* entryGladiator = new p_Gladiator;
					*entryGladiator = *static_cast<p_Gladiator*>(targetUserData->m_object);
					entry.m_target = entryGladiator;

					m_bulletCollisionEntries.push_back(entry);
					break;
				}
				default:
					break;
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
	b2Filter b2Filters[entityIndexes::ci_Count];
	Physics();
	~Physics();

	float64 updateTimer;
	void reset();
	void update(float64 timeStep = PHYSICS_TIMESTEP);
	void createPlatform(std::vector<glm::vec2> platform, unsigned type);
	void setGladiatorCollideLightPlatforms(unsigned gladiatorID, bool collide);
	void addGladiatorWithID(glm::vec2* position, glm::vec2* velocity, uint32_t id);
	void applyForceToGladiator(glm::vec2 direction, unsigned id);
	void applyImpulseToGladiator(glm::vec2 direction, unsigned id);
	void applyGrenadeExplosionToGladiator(glm::vec2* origin, glm::vec2* target, unsigned id);
	void applyExplosionToGladiator(glm::vec2* origin, glm::vec2* target, float constant, unsigned id);
	glm::vec2 getGladiatorVelocity(unsigned id);
	glm::vec2 getGladiatorPosition(unsigned id);
	float getGladiatorMass(unsigned id);
	bool checkIfGladiatorCollidesPlatform(unsigned id);
	int checkIfGladiatorCollidesLadder(unsigned id);
	//void addCollisionCallback(CollisionCallback callback);
	//void removeCollisionCallback(CollisionCallback callback);

	void setGladiatorPosition(glm::vec2 position, unsigned id);
	void removeGladiator(unsigned id);
	
	void addBulletWithID(glm::vec2* position, glm::vec2 impulse, float angle, unsigned shooterID, uint8_t bulletID);
	void addGrenadeWithID(glm::vec2* position, glm::vec2 impulse, unsigned shooterID, uint8_t bulletID);
	void addExplosionWithID(glm::vec2* position, float radius, unsigned shooterID, uint8_t bulletID);
	float32 getEntityRotation(unsigned id);
	float32 getClientSideEntityRotation(unsigned id);
	void removeEntity(uint8_t id);

	ContactListener m_ContactListener;

	std::vector<BulletHit> hitVector;
	std::vector<p_entity*> m_entityVector;

	// Clientside physics, move at some point to a separate file.
	// Use separate ids.
	void addMagazine(glm::vec2* position, glm::vec2 impulse, uint8_t id);
	void removeClientSideEntity(uint8_t id);
	
	std::vector<p_entity*> m_clientEntityVector;
	// Clientside physics end.

private:
	p_entity* getClientSideEntity(uint8_t id);
	//ArenaContactListener m_listener;
	p_entity* getEntity(uint8_t id);
	std::vector<CollisionCallback> m_callbacks;
	int16 gladiatorIdToGroupId(uint32_t playerId);

	b2World* m_b2DWorld;

	std::vector<p_Platform*> m_platformVector;
};
