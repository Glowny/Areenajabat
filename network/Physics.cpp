#if defined(ARENA_SERVER)
#include "Physics.h"


Physics::Physics() 
{

	m_b2DWorld = new b2World(b2Vec2(0,9.81));
	m_b2DWorld->SetContactListener(&m_ContactListener);
};
Physics::~Physics() {};

void Physics::update()
{
	float timeStep = 1.0f / 60.0f;
	int32 velocityIterations = 6;
	int32 positionIterations = 2;

	m_b2DWorld->Step(timeStep, velocityIterations, positionIterations);

	for (unsigned i = 0; i < m_bulletVector.size(); i++)
	{
		if (m_bulletVector[i]->m_contact == true)
		{
			switch (m_bulletVector[i]->m_contactBody)
			{
			case B_Platform:
			{
				printf("PLATFORM HIT O HUMANITY\n");
			}
			break;
			case B_Gladiator:
			{
				printf("GLADIATOR HIT O HUM ANIT\n");
			}
			break;
			default:
				break;
			}
			m_bulletVector[i]->m_contact == false;
		}
		//glm::vec2 position;
		//position.x = m_bulletVector[0]->m_body->GetPosition().x;
		//position.y = m_bulletVector[0]->m_body->GetPosition().y;
		//printf("%f, %f\n", position.x, position.y);
	}
};

void Physics::createPlatform(std::vector<glm::vec2> platform)
{
	b2Vec2* points = new b2Vec2[platform.size()];
	for (unsigned i = 0; i < platform.size(); i++)
	{
		points[i].Set(platform[i].x, platform[i].y);
	}

	p_Platform* temp_platform = new p_Platform;
	unsigned index = m_platformVector.size();
	m_platformVector.push_back(temp_platform);
	m_platformVector[index]->m_shape.CreateChain(points, platform.size());
	m_platformVector[index]->m_bodydef.type = b2_staticBody;
	m_platformVector[index]->m_bodydef.position.Set(0, 0);
	m_platformVector[index]->m_body = m_b2DWorld->CreateBody(&m_platformVector[index]->m_bodydef);
	m_platformVector[index]->m_fixtureDef.shape = &m_platformVector[index]->m_shape;
	m_platformVector[index]->m_fixtureDef.density = 1.0f;
	m_platformVector[index]->m_fixtureDef.friction = 0.3f;
	m_platformVector[index]->m_body->CreateFixture(&m_platformVector[index]->m_fixtureDef);
	m_platformVector[index]->m_body->SetUserData(&types[B_Platform]);
}

// returns id.
unsigned Physics::addGladiator(glm::vec2 position)
{
	p_Gladiator glad;
	glad.m_id = m_gladiatorVector.size();
	b2BodyDef bodydef;
	bodydef.type = b2_dynamicBody;
	bodydef.position.Set(position.x, position.y);
	glad.m_body = m_b2DWorld->CreateBody(&bodydef);

	b2PolygonShape dynamicBox;
	dynamicBox.SetAsBox(32.0f, 32.0f);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &dynamicBox;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.3f;

	b2MassData data;
	data.mass = 1;
	data.center = b2Vec2(16, 64);

	glad.m_body->SetMassData(&data);

	glad.m_body->CreateFixture(&fixtureDef);
	glad.m_body->SetUserData(&types[B_Gladiator]);

	m_gladiatorVector.push_back(glad);
	return glad.m_id;
};

void Physics::moveGladiator(glm::vec2 direction, unsigned id)
{
	m_gladiatorVector[id].m_body->ApplyForce(b2Vec2(direction.x, direction.y),
		m_gladiatorVector[id].m_body->GetWorldCenter(), 1);
}
glm::vec2 Physics::getGladiatorPosition(unsigned id)
{
	b2Vec2 pos = m_gladiatorVector[id].m_body->GetPosition();
	glm::vec2 position;
	position.x = pos.x;
	position.y = pos.y;
	return position;
}
glm::vec2 Physics::getGladiatorVelocity(unsigned id)
{
	b2Vec2 vel = m_gladiatorVector[id].m_body->GetLinearVelocity();
	glm::vec2 velocity;
	velocity.x = vel.x;
	velocity.y = vel.y;
	return velocity;
}
void Physics::removeGladiator(unsigned id)
{

};
void Physics::addBullet(glm::vec2 position, glm::vec2 velocity)
{
	

	b2Vec2 pos(position.x, position.y), vel(velocity.x, velocity.y);
	b2BodyDef bulletBodyDef;
	bulletBodyDef.type = b2_dynamicBody;
	bulletBodyDef.position.Set(pos.x, pos.y);
	b2Body* body = m_b2DWorld->CreateBody(&bulletBodyDef);

	b2PolygonShape dynamicBox;
	dynamicBox.SetAsBox(5.0f, 5.0f);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &dynamicBox;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.3f;

	b2MassData data;
	data.mass = 0.05;
	data.center = b2Vec2(2.5, 2.5);

	body->SetMassData(&data);
	body->CreateFixture(&fixtureDef);

	p_Bullet* bullet = new p_Bullet;
	bullet->m_body = body;
	bullet->m_contact = false;
	bullet->m_contactBody = B_NONE;
	body->SetUserData(&types[B_Bullet]);
	bullet->m_body->ApplyForce(b2Vec2(9000,0), b2Vec2(2.5,2.5), true);
	m_bulletVector.push_back(bullet);

};
void Physics::removeBullet()
{

};
#endif