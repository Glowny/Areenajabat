#if defined(ARENA_SERVER)
#include "Physics.h"


Physics::Physics() 
{
	m_b2DWorld = new b2World(b2Vec2(0,9.81));
};
Physics::~Physics() {};

void Physics::update()
{
	float timeStep = 1.0f / 60.0f;
	int32 velocityIterations = 6;
	int32 positionIterations = 2;

	m_b2DWorld->Step(timeStep, velocityIterations, positionIterations);
};

void Physics::createPlatform(vec2 position,unsigned pointAmount, vec2 platformPoints[])
{
	b2Vec2* points = new b2Vec2[pointAmount];
	for (unsigned i = 0; i < pointAmount; i++)
	{
		points[i].Set(platformPoints[i].x, platformPoints[i].y);
	}

	Platform temp_platform;
	temp_platform.m_shape.CreateChain(points, pointAmount);
	temp_platform.m_bodydef.type = b2_staticBody;
	temp_platform.m_bodydef.position.Set(position.x, position.y);
	temp_platform.m_body = m_b2DWorld->CreateBody(&temp_platform.m_bodydef);
	temp_platform.m_fixtureDef.shape = &temp_platform.m_shape;
	temp_platform.m_fixtureDef.density = 1.0f;
	temp_platform.m_fixtureDef.friction = 0.3f;
	temp_platform.m_body->CreateFixture(&temp_platform.m_fixtureDef);

	m_platformVector.push_back(temp_platform);
}

// returns id.
unsigned Physics::addGladiator(float position_x, float position_y)
{
	p_Gladiator glad;
	glad.m_id = m_gladiatorVector.size();
	b2BodyDef bodydef;
	bodydef.type = b2_dynamicBody;
	bodydef.position.Set(position_x, position_y);
	glad.m_body = m_b2DWorld->CreateBody(&bodydef);

	b2PolygonShape dynamicBox;
	dynamicBox.SetAsBox(1.0f, 1.0f);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &dynamicBox;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.3f;

	b2MassData data;
	data.mass = 1;
	data.center = b2Vec2(20, 20);

	glad.m_body->SetMassData(&data);

	glad.m_body->CreateFixture(&fixtureDef);

	m_gladiatorVector.push_back(glad);

	return glad.m_id;
};

void Physics::moveGladiator(float direction_x, float direction_y, unsigned id)
{
	m_gladiatorVector[id].m_body->ApplyForce(b2Vec2(direction_x, direction_y),
		m_gladiatorVector[id].m_body->GetWorldCenter(), 1);
}
vec2 Physics::getGladiatorPosition(unsigned id)
{
	b2Vec2 pos = m_gladiatorVector[id].m_body->GetPosition();
	vec2 position;
	position.x = pos.x;
	position.y = pos.y;
	return position;
}
vec2 Physics::getGladiatorVelocity(unsigned id)
{
	b2Vec2 vel = m_gladiatorVector[id].m_body->GetLinearVelocity();
	vec2 velocity;
	velocity.x = vel.x;
	velocity.y = vel.y;
	return velocity;
}

void Physics::removeGladiator(unsigned id)
{

};
void Physics::addBullet()
{

};
void Physics::removeBullet()
{

};
#endif