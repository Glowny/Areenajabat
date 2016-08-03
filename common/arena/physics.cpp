#include "physics.h"

//void ArenaContactListener::BeginContact(b2Contact* contact) {
//	b2Body* const bodyA = contact->GetFixtureA()->GetBody();
//	b2Body* const bodyB = contact->GetFixtureB()->GetBody();
//
//	void* userDataA = bodyA->GetUserData();
//	void* userDataB = bodyB->GetUserData();
//
//	(void)userDataA;
//	(void)userDataB;
//
//	(void)bodyA;
//	(void)bodyB;
//}
//
//void ArenaContactListener::EndContact(b2Contact* contact) {
//	(void)contact;
//
//	return;
//}

Physics::Physics() : m_ContactListener(&m_gladiatorVector)
{
	updateTimer = 0;
	m_b2DWorld = new b2World(b2Vec2(0.f,9.81f));
	m_b2DWorld->SetContactListener(&m_ContactListener);

	currentFreeId = 0;

	b2Filter filter;
	// Platform collide filter 
	filter.categoryBits = c_Platform;
	filter.maskBits = c_GladiatorNoCollide | c_Gladiator | c_Bullet;
	filter.groupIndex =0 ;
	b2Filters[ci_Platform] = filter;

	// Light platform collide filter 
	filter.categoryBits = c_LightPlatform;
	filter.maskBits =  c_Gladiator;
	filter.groupIndex =0 ;
	b2Filters[ci_LightPlatform] = filter;

	// ladder
	filter.categoryBits = c_Ladder;
	filter.maskBits = c_GladiatorNoCollide |c_Gladiator;
	filter.groupIndex = 0;
	b2Filters[c_Ladder] = filter;

	// Gladiator collide filter 
	filter.categoryBits = c_Gladiator;
	filter.maskBits = c_Ladder | c_Platform | c_LightPlatform | c_Bullet;
	filter.groupIndex = 0;
	b2Filters[ci_Gladiator] = filter;

	//Gladiator no collide filter
	filter.categoryBits = c_GladiatorNoCollide;
	filter.maskBits =  c_Ladder | c_Platform | c_Bullet;
	filter.groupIndex =0 ;
	b2Filters[ci_GladiatorNoCollide] = filter;

	// Bullet filter. DONE
	filter.categoryBits = c_Bullet;
	filter.maskBits = c_Platform | c_GladiatorNoCollide | c_Gladiator;
	filter.groupIndex = 0;
	b2Filters[ci_Bullet] = filter;
};
Physics::~Physics() {};

void Physics::update(float32 timeStep)
{
	const int32 VelocityIterations = 6;
	const int32 PositionIterations = 2;

	m_b2DWorld->Step(timeStep, VelocityIterations, PositionIterations);

	// Set gladiator positions.
	for (unsigned i = 0; i < m_gladiatorVector.size(); i++)
	{
		b2Vec2 pos = m_gladiatorVector[i]->m_body->GetPosition();
		*m_gladiatorVector[i]->gamePosition = glm::vec2(pos.x * 100.0f, pos.y * 100.0f);
	}

	for (unsigned i = 0; i < m_bulletVector.size(); i++)
	{
		b2Vec2 position = m_bulletVector[i]->m_body->GetPosition();;
		*m_bulletVector[i]->gamePosition = glm::vec2(position.x* 100, position.y*100);
	}

	#pragma region Old impl
	// TODO: old impl, see contact listener.
	//for (unsigned i = 0; i < m_bulletVector.size(); i++)
	//{
	//	b2Vec2 pos = m_bulletVector[i]->m_body->GetPosition();
	//	*m_bulletVector[i]->gamePosition = glm::vec2(pos.x * 100.0f, pos.y * 100.0f);
	//	if (m_bulletVector[i]->m_contact == true)
	//	{
	//		switch (m_bulletVector[i]->m_contactUserData->m_bodyType)
	//		{
	//		case B_Platform:
	//		{
	//			BulletHit hit;
	//			hit.hitType = B_Platform;
	//			hit.position = m_bulletVector[i]->hitPosition;
	//			hitVector.push_back(hit);
	//		}
	//		break;
	//		case B_Gladiator:
	//		{

	//			BulletHit hit;
	//			p_Gladiator* glad = static_cast<p_Gladiator*>(m_bulletVector[i]->m_contactUserData->m_object);
	//			unsigned targetID = static_cast<p_Gladiator*>(m_bulletVector[i]->m_contactUserData->m_object)->m_id;
	//			hit.hitType = B_Gladiator;
	//			hit.position = m_bulletVector[i]->hitPosition;
	//			hit.targetPlayerId = targetID;
	//			hit.shooterPlayerId = static_cast<p_Bullet*>(m_bulletVector[i]->m_myUserData->m_object)->m_shooterID;
	//			hitVector.push_back(hit);
	//		}
	//		break;
	//		default:
	//			break;
	//		}


	//	}
		
		//printf("%f, %f\n", position.x, position.y);
	// }
	
	//if( m_bulletVector.size() != 0)
	//{ 
	//	for (int i = m_bulletVector.size()-1; i >= 0; i--)
	//	{
	//		if (m_bulletVector[i]->m_contact == true)
	//		{
	//			m_bulletVector[i]->m_body->GetWorld()->DestroyBody(m_bulletVector[i]->m_body);
	//			delete(m_bulletVector[i]->m_myUserData);
	//			delete(m_bulletVector[i]);
	//			m_bulletVector.erase(m_bulletVector.begin() + i);
	//		}
	//	}
	//}
#pragma endregion
};

void Physics::createPlatform(std::vector<glm::vec2> platform, unsigned type)
{
	//TEMP LADDER BREAK THINGS
	if (type == 2)
		return;
	b2Vec2* points = new b2Vec2[platform.size()];
	for (unsigned i = 0; i < platform.size(); i++)
	{
		points[i].Set(platform[i].x/100.0f, platform[i].y/100.0f);
	}
	p_userData* userData = new p_userData;
	p_Platform* temp_platform = new p_Platform;
	temp_platform->m_type = B_Platform;
	userData->m_bodyType = B_Platform;
	userData->m_object = temp_platform;


	unsigned index = uint32_t(m_platformVector.size());
	m_platformVector.push_back(temp_platform);
	m_platformVector[index]->m_shape.CreateChain(points, uint32(platform.size()));
	m_platformVector[index]->m_bodydef.type = b2_staticBody;
	m_platformVector[index]->m_bodydef.position.Set(0, 0);
	m_platformVector[index]->m_body = m_b2DWorld->CreateBody(&m_platformVector[index]->m_bodydef);
	m_platformVector[index]->m_fixtureDef.shape = &m_platformVector[index]->m_shape;
	m_platformVector[index]->m_fixtureDef.density = 1.0f;
	m_platformVector[index]->m_fixtureDef.friction = 0.3f;
	switch(type)
	{ B_Gladiator;
		case 0:
			m_platformVector[index]->m_fixtureDef.filter = b2Filters[ci_Platform];
			break;
		case 1:
			m_platformVector[index]->m_fixtureDef.filter = b2Filters[ci_LightPlatform];
			break;
		case 2:
		{
			m_platformVector[index]->m_fixtureDef.filter = b2Filters[ci_Ladder];
			m_platformVector[index]->m_fixtureDef.isSensor = true;
			break;
		}
	}	
	

	m_platformVector[index]->m_body->CreateFixture(&m_platformVector[index]->m_fixtureDef);
	temp_platform->m_userData = userData;
	temp_platform->m_body->SetUserData(userData);
}

// returns id.
unsigned Physics::addGladiator(glm::vec2* position)
{
	p_Gladiator* glad = new p_Gladiator;
	glad->m_id = uint32_t(m_gladiatorVector.size());
	glad->m_type = B_Gladiator;
	glad->gamePosition = position;

	b2BodyDef bodydef;
	bodydef.type = b2_dynamicBody;
	bodydef.position.Set(position->x/100.0f, position->y/100.0f);
	bodydef.fixedRotation = true;
	glad->m_body = m_b2DWorld->CreateBody(&bodydef);

	b2PolygonShape dynamicBox;
	dynamicBox.SetAsBox(0.2f, 0.6f);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &dynamicBox;
	fixtureDef.density = 2.5f;
	fixtureDef.friction = 0.8f;
	fixtureDef.filter = b2Filters[ci_Gladiator];
	
	fixtureDef.filter.groupIndex = gladiatorIdToGroupId(glad->m_id);

	glad->m_body->CreateFixture(&fixtureDef);

	b2MassData data;
	data.mass = 70;
	data.center = b2Vec2(0.2f, 0.45f);
	glad->m_body->SetMassData(&data);
	
	p_userData* userData = new p_userData;
	userData->m_bodyType = B_Gladiator;
	userData->m_object = glad;
	
	glad->m_userData = userData;
	glad->m_body->SetUserData(userData);
	
	
	m_gladiatorVector.push_back(glad);
	
	return glad->m_id;
};

void Physics::setGladiatorCollideLightPlatforms(unsigned gladiatorID, bool collide)
{
	b2Filter filter;
	if (collide)
		filter = b2Filters[ci_Gladiator];
	else
		filter = b2Filters[ci_GladiatorNoCollide];

	filter.groupIndex = gladiatorIdToGroupId(gladiatorID);
	m_gladiatorVector[gladiatorID]->m_body->GetFixtureList()->SetFilterData(filter);
}

void Physics::applyForceToGladiator(glm::vec2 direction, unsigned id)
{
	m_gladiatorVector[id]->m_body->ApplyForce(b2Vec2(direction.x, direction.y),
		m_gladiatorVector[id]->m_body->GetWorldCenter(), 1);
}

void Physics::applyImpulseToGladiator(glm::vec2 direction, unsigned id)
{
	m_gladiatorVector[id]->m_body->ApplyLinearImpulse(b2Vec2(direction.x, direction.y ),
		m_gladiatorVector[id]->m_body->GetWorldCenter(), 1);
}

void Physics::setGladiatorPosition(unsigned id, glm::vec2 position)
{
	b2Vec2 pos;
	pos.x = position.x / 100.0f;
	pos.y = position.y / 100.0f;
	m_gladiatorVector[id]->m_body->SetTransform(pos, m_gladiatorVector[id]->m_body->GetAngle());
}
glm::vec2 Physics::getGladiatorPosition(unsigned id)
{
	b2Vec2 pos = m_gladiatorVector[id]->m_body->GetPosition();
	glm::vec2 position;
	position.x = pos.x*100.0f;
	position.y = pos.y*100.0f;
	return position;
}

bool Physics::checkIfGladiatorCollidesPlatform(unsigned id)
{
	// Is there possibility of multiple edges?
	b2ContactEdge* edge =  m_gladiatorVector[id]->m_body->GetContactList();
	if (edge == NULL)
		return false;
	p_userData* data = static_cast<p_userData*>(edge->contact->GetFixtureA()->GetBody()->GetUserData());
	if (data->m_bodyType == B_Platform)
		return true;
	else
		return false;
}

bool Physics::checkIfGladiatorCollidesLadder(unsigned id)
{
	// Is there possibility of multiple edges?
	b2ContactEdge* edge = m_gladiatorVector[id]->m_body->GetContactList();
	if (edge == NULL)
		return false;
	p_userData* data = static_cast<p_userData*>(edge->contact->GetFixtureA()->GetBody()->GetUserData());
	if (data->m_bodyType == B_Ladder)
		return true;
	else
		return false;
}

//void Physics::addCollisionCallback(CollisionCallback callback) 
//{
//	for (auto it = m_callbacks.begin(); it != m_callbacks.end(); it++) 
//	{
//		if (it->template target<void(arena::NetworkEntity* const, arena::NetworkEntity* const)>() == 
//		    callback.template target<void(arena::NetworkEntity* const, arena::NetworkEntity* const)>())
//		{
//			return;
//		}
//	}
//
//	m_callbacks.push_back(callback);
//}
//void Physics::removeCollisionCallback(CollisionCallback callback) 
//{
//	if (m_callbacks.empty()) return;
//	
//	for (auto it = m_callbacks.begin(); it != m_callbacks.end(); it++)
//	{
//		if (it->template target<void(arena::NetworkEntity* const, arena::NetworkEntity* const)>() ==
//			callback.template target<void(arena::NetworkEntity* const, arena::NetworkEntity* const)>())
//		{
//			m_callbacks.erase(it);
//
//			return;
//		}
//	}
//}

glm::vec2 Physics::getGladiatorVelocity(unsigned id)
{
	b2Vec2 vel = m_gladiatorVector[id]->m_body->GetLinearVelocity();
	glm::vec2 velocity;
	velocity.x = vel.x*100.0f;
	velocity.y = vel.y*100.0f;
	return velocity;
}
void Physics::removeGladiator(unsigned id)
{
    BX_UNUSED(id);
	// TODO: Do proper removal.
};
uint8_t Physics::addBullet(glm::vec2* position, glm::vec2 velocity, unsigned shooterID)
{
	
	b2Vec2 pos(position->x/100.0f, position->y/100.0f), vel(velocity.x/100.0f, velocity.y/100.0f);
	b2BodyDef bulletBodyDef;
	bulletBodyDef.type = b2_dynamicBody;
	bulletBodyDef.position.Set(pos.x, pos.y);
	bulletBodyDef.bullet = true;
	b2Body* body = m_b2DWorld->CreateBody(&bulletBodyDef);

	b2PolygonShape dynamicBox;
	dynamicBox.SetAsBox(0.02f, 0.02f);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &dynamicBox;
	fixtureDef.density = 2.0f;
	fixtureDef.friction = 1.01f;
	fixtureDef.filter = b2Filters[ci_Bullet];
	fixtureDef.filter.groupIndex = gladiatorIdToGroupId(shooterID);

	b2MassData data;
	data.mass = 0.01f;
	data.center = b2Vec2(0.01f, 0.01f);

	body->SetMassData(&data);
	body->CreateFixture(&fixtureDef);

	p_Bullet* bullet = new p_Bullet;
	bullet->bulletId = getFreeBulletId();
	p_userData* userData = new p_userData;
	userData->m_bodyType = B_Bullet;
	bullet->m_type = B_Bullet;
	userData->m_object = bullet;
	bullet->m_myUserData = userData;
	bullet->m_body = body;
	bullet->m_contact = false;
	bullet->m_contactBody = B_NONE;
	bullet->m_shooterID = shooterID;
	bullet->gamePosition = position;
	body->SetUserData(userData);
	bullet->m_body->ApplyLinearImpulse(vel, b2Vec2(1,1), true);
	m_bulletVector.push_back(bullet);
	
	return bullet->bulletId;
};


void Physics::reset()
{
	for (unsigned i = 0; i < m_gladiatorVector.size(); i++)
	{
		m_gladiatorVector[i]->m_body->GetWorld()->DestroyBody(m_gladiatorVector[i]->m_body);
		free(m_gladiatorVector[i]->m_userData);
	}
	for (unsigned i = 0; i < m_bulletVector.size(); i++)
	{
		m_bulletVector[i]->m_body->GetWorld()->DestroyBody(m_bulletVector[i]->m_body);
		delete(m_bulletVector[i]->m_myUserData);
		delete(m_bulletVector[i]);

	}
	m_gladiatorVector.clear();
	m_bulletVector.clear();
}
void Physics::removeBullet(uint8_t id)
{
	// TODO: Do proper removal.
	for (unsigned i = 0; i < m_bulletVector.size(); i++)
	{
		if (id == m_bulletVector[i]->bulletId)
		{ 
			m_bulletVector[i]->cleanUp();
			delete m_bulletVector[i];
			m_bulletVector.erase(m_bulletVector.begin() + i);
			break;
		}
	}
	assert(id < 256);
	isIdFree[id] = true;
}

uint8_t Physics::getFreeBulletId()
{
	nextUint8_t(currentFreeId);
	return currentFreeId;
}

void Physics::nextUint8_t(uint8_t& current)
{
	uint8_t old = current;
	current++;

	// search for id till there is a free one left. Will crash if there is none left.
	while(current != old)
	{ 
		
		if (current > 255)
			current = 0;
		if (isIdFree[current])
		{
			isIdFree[current] = false;			
			return;
		}
		current++;
	}
	// If this happens, there is no free ids left. To add more bullets, use bigger data type
	assert(false);
}

int16 Physics::gladiatorIdToGroupId(uint32_t playerId)
{
	return -(int16)(playerId + 1);
}
