#pragma once
#if defined(ARENA_SERVER)
#include <Box2D\Box2D.h>
#include <stdarg.h>
#include <vector>
#include <glm\glm.hpp>

// TODO: platform has extra stuff that could be removed.
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
	b2Body* m_body;
	void startContact() { m_contact = true; }
	void endContact() { m_contact = false; }
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

private:
	b2World* m_b2DWorld;
	std::vector<p_Gladiator> m_gladiatorVector;
	std::vector<p_Platform*> m_platformVector;
	std::vector<p_Bullet> m_bulletVector;
};
#endif