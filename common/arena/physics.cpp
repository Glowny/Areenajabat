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

Physics::Physics() : m_ContactListener(&m_entityVector)
{
	updateTimer = 0;
	m_b2DWorld = new b2World(b2Vec2(0.f,9.81f));
	m_b2DWorld->SetContactListener(&m_ContactListener);


	b2Filter filter;
	// Platform collide filter 
	filter.categoryBits = c_Platform;
	filter.maskBits = c_GladiatorNoCollide | c_Gladiator | c_GladiatorJumpCollider | c_Bullet | c_Platform | c_Grenade;
	filter.groupIndex = 0 ;
	b2Filters[ci_Platform] = filter;

	// Light platform collide filter 
	filter.categoryBits = c_LightPlatform;
	filter.maskBits =  c_Gladiator| c_GladiatorJumpCollider;
	filter.groupIndex = 0 ;
	b2Filters[ci_LightPlatform] = filter;

	// ladder
	filter.categoryBits = c_Ladder;
	filter.maskBits = c_GladiatorNoCollide |c_Gladiator;
	filter.groupIndex = 0;
	b2Filters[ci_Ladder] = filter;

	// Gladiator collide filter 
	filter.categoryBits = c_Gladiator;
	filter.maskBits = c_Ladder | c_Platform | c_LightPlatform | c_Bullet | c_BulletSensor;
	filter.groupIndex = 0;
	b2Filters[ci_Gladiator] = filter;

	//Gladiator no collide filter
	filter.categoryBits = c_GladiatorNoCollide;
	filter.maskBits =  c_Ladder | c_Platform | c_Bullet | c_BulletSensor;
	filter.groupIndex = 0;
	b2Filters[ci_GladiatorNoCollide] = filter;

	filter.categoryBits = c_GladiatorJumpCollider;
	filter.maskBits = c_Platform | c_LightPlatform;
	filter.groupIndex = 0;
	b2Filters[ci_GladiatorJumpCollider] = filter;

	// Bullet filter.
	// Bullet only collides with platforms. 
	// It passes gladiators, and hits to gladiators are registered by bullet sensor.
	
	filter.categoryBits = c_Bullet;
	filter.maskBits = c_Platform;
	filter.groupIndex = 0;
	b2Filters[ci_Bullet] = filter;

	// Bullet sensor filter.
	filter.categoryBits = c_BulletSensor;
	filter.maskBits = c_GladiatorNoCollide | c_Gladiator;
	filter.groupIndex = 0;
	b2Filters[ci_BulletSensor] = filter;

	// Grenade filter.
	filter.categoryBits = c_Grenade;
	filter.maskBits = c_Platform;
	filter.groupIndex = 0;
	b2Filters[ci_Grenade] = filter;
};
Physics::~Physics() {};

void Physics::update(float64 timeStep)
{
	const int32 VelocityIterations = 8;
	const int32 PositionIterations = 3;

	m_b2DWorld->Step(timeStep, VelocityIterations, PositionIterations);

	// Set Entities positions.
	for (unsigned i = 0; i < m_entityVector.size(); i++)
	{
		if (m_entityVector[i]->m_type == bodyType::B_Gladiator)
		{
			p_Gladiator* glad = static_cast<p_Gladiator*>(m_entityVector[i]);
			b2Vec2 pos = glad->m_body->GetPosition();
			*glad->m_gamePosition = glm::vec2(pos.x * 100.0f, pos.y * 100.0f);
			b2Vec2 velocity = glad->m_body->GetLinearVelocity();
			*glad->m_gamevelocity = glm::vec2(velocity.x * 100.0f, velocity.y * 100.0f);

		}
		if (m_entityVector[i]->m_type == bodyType::B_Bullet || m_entityVector[i]->m_type == bodyType::B_Grenade)
		{
			p_Bullet* bullet = static_cast<p_Bullet*>(m_entityVector[i]);
			b2Vec2 position = bullet->m_body->GetPosition();
			*bullet->gamePosition = glm::vec2(position.x * 100, position.y * 100);
		}
		
	}
	for (unsigned i = 0; i < m_clientEntityVector.size(); i++)
	{
		if (m_clientEntityVector[i]->m_type == bodyType::B_Grenade)
		{
			p_Bullet* bullet = static_cast<p_Bullet*>(m_clientEntityVector[i]);
			b2Vec2 position = bullet->m_body->GetPosition();
			*bullet->gamePosition = glm::vec2(position.x * 100, position.y * 100);
		}
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
	// If there is only single point, do not create platform
	// This can happen if editor is retard.
	if (platform.size() == 1)
		return;
	b2Vec2* points = new b2Vec2[platform.size()];
	for (unsigned i = 0; i < platform.size(); i++)
	{
		points[i].Set(platform[i].x/100.0f, platform[i].y/100.0f);
	}
	p_userData* userData = new p_userData;
	p_Platform* temp_platform = new p_Platform;
	temp_platform->m_type = B_Platform;
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
	{ 
		
		case 0:
			m_platformVector[index]->m_fixtureDef.filter = b2Filters[ci_Platform];
			userData->m_bodyType = B_Platform;
			break;
		case 1:
			m_platformVector[index]->m_fixtureDef.filter = b2Filters[ci_LightPlatform];
			userData->m_bodyType = B_Platform;
			break;
		case 2:
		{
			// Ladder left.
			m_platformVector[index]->m_fixtureDef.filter = b2Filters[ci_Ladder];
			userData->m_bodyType = B_LadderLeft;
			m_platformVector[index]->m_fixtureDef.isSensor = true;
			break;
		}
		case 3:
		{
			// Ladder right.
			m_platformVector[index]->m_fixtureDef.filter = b2Filters[ci_Ladder];
			userData->m_bodyType = B_LadderRight;
			m_platformVector[index]->m_fixtureDef.isSensor = true;
			break;
		}
	}	
	

	m_platformVector[index]->m_body->CreateFixture(&m_platformVector[index]->m_fixtureDef);
	temp_platform->m_userData = userData;
	temp_platform->m_body->SetUserData(userData);
}


void Physics::addGladiatorWithID(glm::vec2* position, glm::vec2* velocity, uint32_t id)
{
	p_Gladiator* glad = new p_Gladiator;
	glad->m_id = id;
	glad->m_type = B_Gladiator;
	glad->m_gamePosition = position;
	glad->m_gamevelocity = velocity;
	b2Vec2 gladiatorBodySize(0.2f, 0.6f);

	b2BodyDef bodydef;
	bodydef.type = b2_dynamicBody;
	bodydef.position.Set(position->x/100.0f, position->y/100.0f);
	bodydef.fixedRotation = true;
	glad->m_body = m_b2DWorld->CreateBody(&bodydef);

	b2BodyDef sensorBodydef;
	sensorBodydef.type = b2_dynamicBody;
	sensorBodydef.position.Set(position->x / 100.0f + gladiatorBodySize.x/6.0f, position->y / 100.0f + gladiatorBodySize.y);
	sensorBodydef.fixedRotation = true;
	glad->m_sensorBody = m_b2DWorld->CreateBody(&sensorBodydef);
	

	b2PolygonShape sensorBox;
	b2Vec2 points[4]
	{
		b2Vec2(0.0f,0.0f) , b2Vec2(0.0f, 0.29f), b2Vec2(gladiatorBodySize.x/1.5f, 0.29f), b2Vec2(gladiatorBodySize.x / 1.5f, 0.0f)
	};
	sensorBox.Set(points, 4);

	b2FixtureDef sensorFixtureDef;
	sensorFixtureDef.shape = &sensorBox;
	sensorFixtureDef.isSensor = true;
	sensorFixtureDef.filter = b2Filters[ci_GladiatorJumpCollider];
	glad->m_sensorBody->CreateFixture(&sensorFixtureDef);

	
	b2PolygonShape dynamicBox;
	dynamicBox.SetAsBox(gladiatorBodySize.x, gladiatorBodySize.y);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &dynamicBox;
	//fixtureDef.density = 2.5f;
	fixtureDef.friction = 0.2f; //0.8f;
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
	
	b2WeldJointDef def;
	def.bodyA = glad->m_body;
	def.bodyB = glad->m_sensorBody;
	def.collideConnected = false;
	b2Vec2 anchor = def.bodyB->GetPosition();
	def.Initialize(glad->m_body, glad->m_sensorBody, anchor);
	m_b2DWorld->CreateJoint(&def);


	glad->m_userData = userData;
	glad->m_body->SetUserData(userData);
	glad->m_sensorBody->SetUserData(userData);
	m_entityVector.push_back(glad);
};

void Physics::setGladiatorCollideLightPlatforms(unsigned gladiatorID, bool collide)
{
	// Note: can be used to set any entity filter data with a body.
	b2Filter filter;
	if (collide)
		filter = b2Filters[ci_Gladiator];
	else
		filter = b2Filters[ci_GladiatorNoCollide];

	filter.groupIndex = gladiatorIdToGroupId(gladiatorID);
	getEntity(gladiatorID)->m_body->GetFixtureList()->SetFilterData(filter);
}

void Physics::applyExplosionToGladiator(glm::vec2* origin, glm::vec2* target, const float CONSTANT, unsigned id) {
	float x = target->x - origin->x;
	float y = target->y - origin->y;
	x = abs(x) / x;
	y = abs(y) / y;
	glm::vec2 direction = glm::vec2(x, y);
	direction = direction * CONSTANT;
	applyImpulseToGladiator(direction, id);
}

void Physics::applyGrenadeExplosionToGladiator(glm::vec2* origin, glm::vec2* target, unsigned id) {
	applyExplosionToGladiator(origin, target, EXPLOSIONIMPULSE, id);
}

void Physics::applyForceToGladiator(glm::vec2 direction, unsigned id)
{
	p_entity* entity = getEntity(id);
	entity->m_body->ApplyForce(b2Vec2(direction.x, direction.y),
		entity->m_body->GetWorldCenter(), 1);
}

void Physics::applyImpulseToGladiator(glm::vec2 direction, unsigned id)
{
	p_entity* entity = getEntity(id);
	entity->m_body->ApplyLinearImpulse(b2Vec2(direction.x, direction.y ),
		entity->m_body->GetWorldCenter(), 1);
}

void Physics::setGladiatorPosition(glm::vec2 position, unsigned id )
{
	b2Vec2 pos;
	pos.x = position.x / 100.0f;
	pos.y = position.y / 100.0f;
	getEntity(id)->m_body->SetTransform(pos, getEntity(id)->m_body->GetAngle());
}
glm::vec2 Physics::getEntityPosition(unsigned id)
{
	b2Vec2 pos = getEntity(id)->m_body->GetPosition();
	glm::vec2 position;
	position.x = pos.x*100.0f;
	position.y = pos.y*100.0f;
	return position;
}

bool Physics::entityExists(unsigned id)
{
	if (getEntity(id) == nullptr)
		return false;
	return true;
}

bool Physics::checkIfGladiatorCollidesPlatform(unsigned id)
{
	// Is there possibility of multiple edges?
	p_entity* entity = getEntity(id);
	b2ContactEdge* edge;
	if (entity->m_type == bodyType::B_Gladiator)
	{
		p_Gladiator* gladiator =	static_cast<p_Gladiator*>(entity);
		edge = gladiator->m_sensorBody->GetContactList();
	}
	else
	{
		edge = entity->m_body->GetContactList();		
	}
	
	if (edge == NULL || !edge->contact->IsTouching())
		return false;
	p_userData* data = static_cast<p_userData*>(edge->contact->GetFixtureA()->GetBody()->GetUserData());
	if (data->m_bodyType == B_Platform)
		return true;
	else
		return false;
}

// Returns zero if does not, 1 if left, 2 if right.
int Physics::checkIfGladiatorCollidesLadder(unsigned id)
{
	// Is there possibility of multiple edges?
	b2ContactEdge* edge = getEntity(id)->m_body->GetContactList();
	while (edge != NULL)
	{
		if (edge->contact->IsTouching())
		{ 
			p_userData* data = static_cast<p_userData*>(edge->contact->GetFixtureA()->GetBody()->GetUserData());
			if (data->m_bodyType == B_LadderLeft)
				return 1;
			else if (data->m_bodyType == B_LadderRight)
				return 2;
		}
		edge = edge->next;
	}
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

void Physics::addMagazine(glm::vec2* position, glm::vec2 impulse, uint8_t id)
{

	b2Vec2 pos(position->x / 100.0f, position->y / 100.0f), imp(impulse.x / 100.0f, impulse.y / 100.0f);
	b2BodyDef magBodyDef;
	magBodyDef.type = b2_dynamicBody;
	magBodyDef.position.Set(pos.x, pos.y);
	//bulletBodyDef.bullet = true;
	b2Body* body = m_b2DWorld->CreateBody(&magBodyDef);

	b2PolygonShape dynamicBox;
	dynamicBox.SetAsBox(0.10f, 0.20f);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &dynamicBox;
	//fixtureDef.density = 0.5f;
	fixtureDef.friction = 0.81f;
	fixtureDef.restitution = 0.2f;
	fixtureDef.density = 3;
	fixtureDef.filter = b2Filters[ci_Grenade];
	fixtureDef.filter.groupIndex = 0;//gladiatorIdToGroupId(shooterID);

	b2MassData data;
	data.center = b2Vec2(0.05f, 0.10f);
	data.mass = 0.05f;
	body->SetMassData(&data);
	body->CreateFixture(&fixtureDef);
	body->SetAngularDamping(3.50f);

	p_Bullet* bullet = new p_Bullet;
	bullet->m_id = id;
	p_userData* userData = new p_userData;
	userData->m_bodyType = B_Grenade;
	bullet->m_type = B_Grenade;
	bullet->m_bulletType = arena::BulletType::GrenadeBullet;
	userData->m_object = bullet;
	bullet->m_myUserData = userData;
	bullet->m_body = body;
	bullet->m_body->SetActive(true);
	bullet->m_contact = false;
	bullet->m_contactBody = B_NONE;
	bullet->gamePosition = position;
	body->SetUserData(userData);
	bullet->m_body->ApplyLinearImpulse(imp, data.center, true);
	m_clientEntityVector.push_back(bullet);
}

glm::vec2 Physics::getEntityVelocity(unsigned id)
{
	b2Vec2 vel = getEntity(id)->m_body->GetLinearVelocity();
	glm::vec2 velocity;
	velocity.x = vel.x*100.0f;
	velocity.y = vel.y*100.0f;
	return velocity;
}
float Physics::getEntityVelocityAngle(unsigned id)
{
	b2Vec2 vel =  getEntity(id)->m_body->GetLinearVelocity();
	float angle = atan2(vel.y, vel.x);
	return angle;
}

float Physics::getGladiatorMass(unsigned id)
{
	return getEntity(id)->m_body->GetMass();
	
}

void Physics::removeGladiator(unsigned id)
{
    BX_UNUSED(id);
	// TODO: Do proper removal.
};
//TODO: at the moment bullet, grenade and explosions are handled by same system. 
// Rename the system or make separate ones if needed.


void Physics::addBulletWithID(glm::vec2* position, glm::vec2 impulse, float angle, unsigned shooterID, uint8_t bulletID)
{

	b2Vec2 pos(position->x / 100.0f, position->y / 100.0f), imp(impulse.x / 100.0f, impulse.y / 100.0f);
	b2BodyDef bulletBodyDef;
	bulletBodyDef.type = b2_dynamicBody;
	bulletBodyDef.position.Set(pos.x, pos.y);
	bulletBodyDef.angle = angle;
	bulletBodyDef.bullet = true;
	b2Body* body = m_b2DWorld->CreateBody(&bulletBodyDef);
	body->SetBullet(true);
	body->SetFixedRotation(false);
	b2PolygonShape dynamicBox;
	dynamicBox.SetAsBox(0.02f, 0.02f);
	b2PolygonShape sensorBox;
	b2Vec2 points[4]
	{
		b2Vec2(1.0f,0.0f) , b2Vec2(1.5f, 0.0f), b2Vec2(1.5f, 0.1f), b2Vec2(1.0f, 0.1f)
	};
	sensorBox.Set(points, 4);

	// Fixture definiton for collisions on platforms or other similiar objects.
	b2FixtureDef physicalFixtureDef;
	physicalFixtureDef.shape = &dynamicBox;
	float rest= rand() % 2;
	rest= rest / 100.0f;
	physicalFixtureDef.restitution = rest;
	physicalFixtureDef.density = 2.0f;
	physicalFixtureDef.friction = 1.01f;
	physicalFixtureDef.filter = b2Filters[ci_Bullet];

	// TODO: Remove this as it is not needed any more (Bullets do not collide with players).
	physicalFixtureDef.filter.groupIndex = gladiatorIdToGroupId(shooterID);

	// Fixture definition for collisions on players (sensor).
	b2FixtureDef sensorFixtureDef;
	sensorFixtureDef.shape = &sensorBox;
	sensorFixtureDef.isSensor = true;
	sensorFixtureDef.filter = b2Filters[ci_BulletSensor];
	sensorFixtureDef.filter.groupIndex = gladiatorIdToGroupId(shooterID);


	b2MassData data;
	data.mass = 0.01f;
	data.center = b2Vec2(0.01f, 0.01f);

	body->SetMassData(&data);
	body->CreateFixture(&physicalFixtureDef);
	body->CreateFixture(&sensorFixtureDef);
	p_Bullet* bullet = new p_Bullet;
	bullet->m_id = bulletID;
	p_userData* userData = new p_userData;
	userData->m_bodyType = B_Bullet;
	bullet->m_type = B_Bullet;
	userData->m_object = bullet;
	bullet->m_bulletType = arena::BulletType::GladiusBullet;
	bullet->m_myUserData = userData;
	bullet->m_body = body;
	bullet->m_contact = false;
	bullet->m_contactBody = B_NONE;
	bullet->m_shooterID = shooterID;
	bullet->gamePosition = position;
	body->SetUserData(userData);
	bullet->m_body->ApplyLinearImpulse(imp, data.center, true);
	m_entityVector.push_back(bullet);

};

void Physics::addGrenadeWithID(glm::vec2* position, glm::vec2 impulse, unsigned shooterID, uint8_t bulletID)
{
	b2Vec2 pos(position->x / 100.0f, position->y / 100.0f), imp(impulse.x / 100.0f, impulse.y / 100.0f);
	b2BodyDef bulletBodyDef;
	bulletBodyDef.type = b2_dynamicBody;
	bulletBodyDef.angle = 0.8f;
	bulletBodyDef.fixedRotation = false;
	bulletBodyDef.position.Set(pos.x, pos.y);
	//bulletBodyDef.bullet = true;
	b2Body* body = m_b2DWorld->CreateBody(&bulletBodyDef);

	b2PolygonShape dynamicBox;
	dynamicBox.SetAsBox(0.14f, 0.29f);
	

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &dynamicBox;

	fixtureDef.friction = 1.0f;
	fixtureDef.density = 2.5;
	fixtureDef.restitution = 0.23f;
	fixtureDef.filter = b2Filters[ci_Grenade];
	fixtureDef.filter.groupIndex = 0;//gladiatorIdToGroupId(shooterID);

	b2MassData data;
	data.center = b2Vec2(0.07f, 0.145);
	data.mass = 0.390f;

	
	body->SetMassData(&data);
	body->CreateFixture(&fixtureDef);
	body->SetAngularDamping(5);
	body->SetLinearDamping(0.2);

	p_Bullet* bullet = new p_Bullet;
	bullet->m_id = bulletID;
	p_userData* userData = new p_userData;
	userData->m_bodyType = B_Grenade;
	bullet->m_type = B_Grenade;
	bullet->m_bulletType = arena::BulletType::GrenadeBullet;
	userData->m_object = bullet;
	bullet->m_myUserData = userData;
	bullet->m_body = body;
	bullet->m_body->SetActive(true);
	bullet->m_contact = false;
	bullet->m_contactBody = B_NONE;
	bullet->m_shooterID = shooterID;
	bullet->gamePosition = position;
	body->SetUserData(userData);
	bullet->m_body->ApplyLinearImpulse(imp, b2Vec2(data.center.x, data.center.y), true);
	m_entityVector.push_back(bullet);

};



float32 Physics::getEntityRotation(unsigned id)
{
	for (unsigned i = 0; i < m_entityVector.size(); i++)
	{
		if (id == m_entityVector[i]->m_id && m_entityVector[i]->m_body != nullptr)
		{
			if (m_entityVector[i]->m_type == bodyType::B_Bullet)
			{
				return getEntityVelocityAngle(id);
			}
			else
			return m_entityVector[i]->m_body->GetAngle();
		}
	}
		printf("SS Trying to get rotation of entity with no body, id: %d\n", id);
		return 0.0f;
}

float32 Physics::getClientSideEntityRotation(unsigned id)
{
	for (unsigned i = 0; i < m_clientEntityVector.size(); i++)
	{
		if (id == m_clientEntityVector[i]->m_id && m_clientEntityVector[i]->m_body != nullptr)
		{
			return m_clientEntityVector[i]->m_body->GetAngle();
		}
	}
	printf("CS Trying to get rotation of entity with no body, id: %d\n", id);
	return 0.0f;
}

void Physics::addExplosionWithID(glm::vec2* position, float radius, unsigned shooterID, uint8_t bulletID)
{
	b2Vec2 pos(position->x / 100.0f, position->y / 100.0f);
	b2BodyDef bulletBodyDef;
	bulletBodyDef.type = b2_kinematicBody;
	bulletBodyDef.position.Set(pos.x, pos.y);
	bulletBodyDef.bullet = false;
	b2Body* body = m_b2DWorld->CreateBody(&bulletBodyDef);

	b2CircleShape circle;
	circle.m_radius = radius/100;

	// Fixture definition for collisions on players (sensor).
	b2FixtureDef sensorFixtureDef;
	sensorFixtureDef.shape = &circle;
	sensorFixtureDef.isSensor = true;
	// We can use the bullet sensor as it reports all collides with gladiators.

	sensorFixtureDef.filter = b2Filters[ci_BulletSensor];
	sensorFixtureDef.filter.groupIndex = gladiatorIdToGroupId(shooterID);

	body->CreateFixture(&sensorFixtureDef);

	p_Bullet* bullet = new p_Bullet;
	bullet->m_id = bulletID;
	p_userData* userData = new p_userData;
	userData->m_bodyType = B_Explosion;
	bullet->m_type = B_Explosion;
	userData->m_object = bullet;
	bullet->m_bulletType = arena::BulletType::GrenadeBullet;
	bullet->m_myUserData = userData;
	bullet->m_body = body;
	bullet->m_contact = false;
	bullet->m_contactBody = B_NONE;
	bullet->m_shooterID = shooterID;
	bullet->gamePosition = position;
	body->SetUserData(userData);
	body->SetAwake(true);

	m_entityVector.push_back(bullet);

}


void Physics::reset()
{
	for (unsigned i = 0; i < m_entityVector.size(); i++)
	{
		m_entityVector[i]->cleanUp();
		delete(m_entityVector[i]);
	}
	m_entityVector.clear();
}
void Physics::removeEntity(uint8_t id)
{
	for (unsigned i = 0; i < m_entityVector.size(); i++)
	{
		if (id == m_entityVector[i]->m_id)
		{ 
			m_entityVector[i]->cleanUp();
			delete m_entityVector[i];
			m_entityVector.erase(m_entityVector.begin() + i);
			break;
		}
	}

}

void Physics::removeClientSideEntity(uint8_t id)
{
	for (unsigned i = 0; i < m_clientEntityVector.size(); i++)
	{
		if (id == m_clientEntityVector[i]->m_id)
		{
			m_clientEntityVector[i]->cleanUp();
			delete m_clientEntityVector[i];
			m_clientEntityVector.erase(m_entityVector.begin() + i);
			break;
		}
	}

}

int16 Physics::gladiatorIdToGroupId(uint32_t playerId)
{
	return -(int16)(playerId + 1);
}

p_entity* Physics::getEntity(uint8_t id)
{
	for (auto entity = m_entityVector.begin(); entity != m_entityVector.end(); entity++)
	{
		if((*entity)->m_id == id)
			return *entity;
	}
    return nullptr;
}

p_entity* Physics::getClientSideEntity(uint8_t id)
{
	for (auto entity = m_clientEntityVector.begin(); entity != m_clientEntityVector.end(); entity++)
	{
		if ((*entity)->m_id == id)
			return *entity;
	}
	return nullptr;
}