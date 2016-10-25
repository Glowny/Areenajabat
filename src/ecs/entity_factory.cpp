#include "entity_factory.h"
#include "entity_builder.h"
#include "transform.h"
#include <cassert>
#include <common/debug.h>
#include "../app.h"
#include "physics_component.h"
#include "sprite_renderer.h"
#include "timer.h"
#include "movement.h"
#include "player_link.h"
#include "bullet_trail.h"
#include "../res/texture_resource.h"
#include "common/arena/gladiator.h"
#include "../ecs/character_component.h"
#include "../ecs/camera_component.h"
#include "../ecs/animator.h"
#include "../res/spriter_resource.h"
glm::vec2 radToVec(float r)
{
	return glm::vec2(cos(r), sin(r));
}
namespace arena
{
	EntityFactory::EntityFactory(Physics* physics, Scene* scene) :m_physics(physics), m_scene(scene)
	{
		m_resources = App::instance().resources();
	}

	Entity* EntityFactory::createProjectile(BulletData &data)
	{
		Entity* entity;
		if (data.m_type == BulletType::GrenadeBullet)
			entity = createGrenade(data);
		else
			entity = createBullet(data);
		return entity;
	}
	Entity* EntityFactory::createBullet(BulletData &data)
	{
		createMuzzleFlash(data.m_position, data.m_rotation);
		EntityBuilder builder;
		builder.begin();
		Transform* transform = builder.addTransformComponent();
		transform->m_position = data.m_position;
		SpriteRenderer* renderer = builder.addSpriteRenderer();
		renderer->setTexture(m_resources->get<TextureResource>(ResourceType::Texture, "bullet_placeholder1.png"));
		renderer->setRotation(data.m_rotation);
		renderer->anchor();

		glm::vec2 vectorAngle = radToVec(data.m_rotation);
		float impulse = 0.0f;
		switch (data.m_type)
		{
		case BulletType::GladiusBullet:
			impulse = GLADIUSIMPULSE;
			break;
		case BulletType::ShardBullet:
			impulse = SHARDIMPULSE;
			break;
		case BulletType::ShotgunBullet:
			impulse = SHOTGUNIMPULSE;
			break;
		}
		glm::vec2 force(vectorAngle.x * impulse, vectorAngle.y * impulse);
		m_physics->addBulletWithID(&transform->m_position, force, data.m_rotation, data.m_ownerId, data.m_id);
		PhysicsComponent* component = builder.addPhysicsComponent();
		component->m_physicsId = data.m_id;

		builder.addIdentifier(EntityIdentification::BulletModel);

		BulletTrail *trail = builder.addBulletTrail();
		trail->bulletId = data.m_id;

		Entity* entity = builder.getResults();
		m_scene->registerEntity(entity);
		return entity;
		
	}
	Entity* EntityFactory::createGrenade(BulletData &data)
	{
		EntityBuilder builder;
		builder.begin();
		Transform* transform = builder.addTransformComponent();
		transform->m_position = data.m_position;

		SpriteRenderer* renderer = builder.addSpriteRenderer();

		renderer->setTexture(m_resources->get<TextureResource>(ResourceType::Texture, "effects/grenade.png"));
		renderer->setRotation(data.m_rotation);
		glm::vec2& origin = renderer->getOrigin();
		origin = glm::vec2(6, 14.5);
	
		renderer->setLayer(4);
		renderer->anchor();

		glm::vec2 vectorAngle = radToVec(data.m_rotation);
		glm::vec2 force(vectorAngle.x * GRENADEIMPULSE, vectorAngle.y * GRENADEIMPULSE);
		m_physics->addGrenadeWithID(&transform->m_position, force,  data.m_ownerId, data.m_id);
		PhysicsComponent* physicsComponent = builder.addPhysicsComponent();
		physicsComponent->m_physicsId = data.m_id;

		Entity* entity = builder.getResults();
		m_scene->registerEntity(entity);
		return entity;
	}
	Entity* EntityFactory::createMuzzleFlash(glm::vec2 position, float aimRotation)
	{
		EntityBuilder builder;
		builder.begin();
		Transform* transform = builder.addTransformComponent();
		SpriteRenderer* renderer = builder.addSpriteRenderer();

		Timer* timer = builder.addTimer();
		timer->m_lifeTime = 0.05f;
		builder.addIdentifier(EntityIdentification::MuzzleFlash);
		renderer->setTexture(m_resources->get<TextureResource>(ResourceType::Texture, "effects/muzzleFlash_ss.png"));
		Rectf& source = renderer->getSource();

		unsigned sprite = rand() % 3;

		source.x = 0.0f + (float)sprite * 32.0f;
		source.y = 0.0f;
		source.w = 32.0f;
		source.h = 32.0f;

		// Bullet entity is updated once before sending, so it's is no in creation position.
		// This should be fixed later, but for now we need to backtrack the bullet position a bit.
		glm::vec2 angleAsVector = radToVec(aimRotation);
		glm::vec2 backTrackedBulletPosition = glm::vec2(position.x + angleAsVector.x * 70, position.y + angleAsVector.y * 70);

		transform->m_position = glm::vec2(backTrackedBulletPosition.x - 16, backTrackedBulletPosition.y - 16);
		glm::vec2& origin = renderer->getOrigin();
		origin.x = origin.x + 16; origin.y = origin.y + 16;
		renderer->setRotation((float32)aimRotation + 3.142f);
		renderer->setLayer(6);
		renderer->anchor();
		Entity* entity = builder.getResults();
		m_scene->registerEntity(entity);
		return entity;
	}
	void EntityFactory::createSmoke(glm::vec2 position, glm::vec2 velocity)
	{
		EntityBuilder builder;
		builder.begin();

		Timer* timer = builder.addTimer();

		// Load gun smoke, randomize rotation and position on spawn.

		for (int i = 0; i < rand() % 5 + 3; i++)
		{
			int spriteX = rand() % 4;

			float rotation = 0;
			int xOffset = 0, yOffset = 0;

			builder.begin();
			builder.addIdentifier(EntityIdentification::Smoke);

			// Timer
			timer = builder.addTimer();
			timer->m_lifeTime = 0.5f;

			// Drawing stuff
			Transform* transform = builder.addTransformComponent();
			SpriteRenderer* renderer = builder.addSpriteRenderer();
			renderer->setLayer(5);
			renderer->setTexture(m_resources->get<TextureResource>(ResourceType::Texture, "effects/gunSmoke1_ss.png"));
			glm::vec2& origin = renderer->getOrigin();
			origin.x = origin.x + 16; origin.y = origin.y + 16;
			renderer->setRotation(float32(rand() % 7));
			Rectf& source = renderer->getSource();
			source.x = 32 * (float)spriteX; source.y = 0; source.w = 32; source.h = 32;

			if (rand() % 2 == 1) {
				xOffset = rand() % 10;
				rotation = (rand() % 3) / 100.0f;
			}
			else {
				xOffset = -(rand() % 10);
				rotation = (float)-(rand() % 3) / 100.0f;
			}
			if (rand() % 2 == 1) {
				yOffset = rand() % 10;
			}
			else {
				yOffset = -(rand() % 10);
			}

			transform->m_position = glm::vec2(position.x + xOffset, position.y + yOffset);

			// Movement
			Movement* movement = builder.addMovement();
			movement->m_velocity = glm::vec2(velocity.x + xOffset / 5, velocity.y + yOffset / 5);
			movement->m_rotationSpeed = rotation;

			renderer->anchor();
			Entity* entity = builder.getResults();
			m_scene->registerEntity(entity);
			
		}
	}
	Entity* EntityFactory::createMagazine(glm::vec2 position, glm::vec2 force, bool flip)
	{
		EntityBuilder builder;
		builder.begin();

		PhysicsComponent* component = builder.addPhysicsComponent();
		component->m_physicsId = getFreeEntityId();
		component->clientSide = true;
		Transform* transform = builder.addTransformComponent();
		transform->m_position = position;
		m_physics->addMagazine(&transform->m_position, glm::vec2(force.x / 100 * -1, force.y / 100 * -1), component->m_physicsId);

		builder.addIdentifier(arena::EntityIdentification::Magazine);
		Timer* timer = builder.addTimer();
		timer->m_lifeTime = 5;
		timer->m_currentTime = 0;
		ResourceManager* resources = App::instance().resources();
		(void)resources;
		SpriteRenderer* renderer = builder.addSpriteRenderer();
		glm::vec2& origin = renderer->getOrigin();
		origin = glm::vec2(7, 15);
		glm::vec2& offSet = renderer->getOffset();
		offSet = glm::vec2(3, -7);
		glm::vec2& scale = renderer->getScale();
		if (flip)
			scale.x = -1;
		renderer->setTexture(resources->get<TextureResource>(ResourceType::Texture, "effects/gladiusClip.png"));
		renderer->setRotation(0.0f);
		renderer->setLayer(3);
		renderer->anchor();

		Entity* entity = builder.getResults();
		m_scene->registerEntity(entity);

		return entity;
	}
	Entity* EntityFactory::createBulletHitBlood(unsigned direction, glm::vec2 position)
	{
		EntityBuilder builder;
		builder.begin();

		glm::vec2 offset = glm::vec2(0.0f, 0.0f);
		float directionX = 1.0f;
		// check if bullet is approaching from right.
		if (direction == 0)
			directionX = -1.0f;

		Transform* transform = builder.addTransformComponent();
		// Set offset to middle of sprite.

		transform->m_position = glm::vec2(position.x + offset.x, position.y + offset.y);

		ResourceManager* resources = App::instance().resources();
		(void)resources;
		SpriteRenderer* renderer = builder.addSpriteRenderer();

		// Get scale and rotate according to direction where hit comes.
		glm::vec2 &scale = renderer->getScale();
		scale.x = scale.x * directionX;

		renderer->setTexture(resources->get<TextureResource>(ResourceType::Texture, "effects/bloodPenetrationAnimation1_ss.png"));
		Rectf rect = renderer->getSource();
		renderer->setSize(128, 32);
		renderer->setLayer(4);
		rect.x = 0; rect.y = 0;
		rect.w = 128; rect.h = 32;
		renderer->anchor();

		Timer* timer = builder.addTimer();
		timer->m_lifeTime = 0.5f;

		Movement* move = builder.addMovement();
		move->m_velocity = glm::vec2(5.0f* directionX, 4.81f);

		builder.addIdentifier(EntityIdentification::HitBlood);
		Entity* entity = builder.getResults();
		m_scene->registerEntity(entity);
		return entity;
	}
	Entity* EntityFactory::createExplosion(glm::vec2 position, float rotation)
	{
		EntityBuilder builder;
		for (int i = 0; i < 6; i++)
		{
			int spriteX = rand() % 4;
			int spriteY = 0;

			if (spriteX == 0)
				spriteY = rand() % 2;

			float smokeRotation = 0;
			int xOffset = 0, yOffset = 0;

			builder.begin();

			builder.addIdentifier(EntityIdentification::GrenadeSmoke);
			// Timer
			Timer* timer = builder.addTimer();
			timer->m_lifeTime = 3.0f;

			// Drawing stuff
			Transform* transform = builder.addTransformComponent();
			SpriteRenderer* renderer = builder.addSpriteRenderer();
			renderer->setTexture(m_resources->get<TextureResource>(ResourceType::Texture, "effects/grenadeSmoke_ss.png"));
			glm::vec2& origin = renderer->getOrigin();
			origin.x = origin.x + 64; origin.y = origin.y + 64;
			renderer->setRotation(float32(rand() % 7));
			renderer->setSize(2048.f, 1024.f);
			renderer->setLayer(5);
			Rectf& source = renderer->getSource();
			source.x = 128 * (float)spriteX; source.y = 128 * (float)spriteY; source.w = 128; source.h = 128;

			if (rand() % 2 == 1) {
				xOffset = rand() % 40;
			}
			else {
				xOffset = -(rand() % 40);
			}
			if (rand() % 2 == 1) {
				yOffset = rand() % 40;
			}
			else {
				yOffset = -(rand() % 40);
			}
			if (rand() % 2 == 1) {
				smokeRotation = 0.001f;
			}
			else {
				smokeRotation = -0.001f;
			}

			//transform->m_position = glm::vec2(bullet->m_position->x+xOffset-16, bullet->m_position->y+yOffset-16);
			transform->m_position = glm::vec2(position.x - 32.0f + xOffset, position.y - 32.0f + yOffset);

			// Movement
			Movement* movement = builder.addMovement();
			//movement->m_velocity = glm::vec2(float(xOffset)/100.0f, float(yOffset) / 100.0f);
			movement->m_velocity = glm::vec2(cos(rotation) * 2 + float(xOffset) / 20.0f, sin(rotation) * 2 + float(yOffset) / 20.0f);
			movement->m_rotationSpeed = smokeRotation;

			renderer->anchor();
			Entity* entity = builder.getResults();
			m_scene->registerEntity(entity);
			return entity;
		}

		builder.begin();
		Timer* timer = builder.addTimer();
		builder.addIdentifier(EntityIdentification::Explosion);
		timer->m_lifeTime = 0.05f;

		// Drawing stuff
		Transform* transform = builder.addTransformComponent();
		SpriteRenderer* renderer = builder.addSpriteRenderer();

		renderer->setTexture(m_resources->get<TextureResource>(ResourceType::Texture, "effects/grenadeFlash.png"));
		glm::vec2& origin = renderer->getOrigin();
		origin.x = origin.x + 64; origin.y = origin.y + 64;
		renderer->setSize(256.f, 256.f);
		renderer->setRotation(float32(rand() % 7));
		renderer->setLayer(6);
		transform->m_position = glm::vec2(position.x - 32.0f, position.y -32.0f);

		renderer->anchor();
		Entity* entity = builder.getResults();
		m_scene->registerEntity(entity);
		return entity;
		
	}
	Entity* EntityFactory::createExplosionBlood(glm::vec2 position)
	{
		EntityBuilder builder;
		builder.begin();
		Timer* timer = builder.addTimer();
		timer->m_lifeTime = 0.5f;
		Transform* transform = builder.addTransformComponent();
		transform->m_position = glm::vec2(position.x - 64, position.y - 64);

		ResourceManager* resources = App::instance().resources();
		(void)resources;
		SpriteRenderer* renderer = builder.addSpriteRenderer();

		renderer->setTexture(resources->get<TextureResource>(ResourceType::Texture, "effects/bloodExplosionAnimation_ss.png"));
		renderer->setSize(1024, 512);
		Rectf rect = renderer->getSource();
		rect.x = 0; rect.y = 0;
		rect.w = 256; rect.h = 256;
		renderer->anchor();
		renderer->setLayer(6);
		Movement* move = builder.addMovement();
		move->m_velocity = glm::vec2(0, 0);

		builder.addIdentifier(EntityIdentification::ExplosionBlood);
		Entity* entity = builder.getResults();
		m_scene->registerEntity(entity);
		return entity;
		
	}
	Entity* EntityFactory::createMiniBomb(uint32_t playerIndex, float time)
	{
		EntityBuilder builder;
		builder.begin();
		Timer* timer = builder.addTimer();
		timer->m_lifeTime = time;
		timer->m_currentTime = 0.0f;
		builder.addIdentifier(arena::EntityIdentification::Minibomb);
		Entity* entity = builder.getResults();
		entity->add(new PlayerLink(playerIndex));
		m_scene->registerEntity(entity);
		return entity;
	}
	Entity* EntityFactory::createBulletHit(BulletHitData& data)
	{
	
		EntityBuilder builder;
		builder.begin();

		Transform* transform = builder.addTransformComponent();
		transform->m_position = data.m_position;

		ResourceManager* resources = App::instance().resources();
		(void)resources;
		SpriteRenderer* renderer = builder.addSpriteRenderer();

		renderer->setTexture(resources->get<TextureResource>(ResourceType::Texture, "effects/gunSmoke1_ss.png"));
		Rectf rect = renderer->getSource();
		renderer->setSize(32, 32);
		rect.x = 0; rect.y = 0;
		rect.w = 32; rect.h = 32;
		renderer->anchor();
		renderer->setLayer(5);
		Timer* timer = builder.addTimer();
		timer->m_lifeTime = 0.5f;

		builder.addIdentifier(EntityIdentification::Smoke);
		Entity* entity = builder.getResults();
		m_scene->registerEntity(entity);
		return entity;
	}

	Entity* EntityFactory::createMousePointer(Camera& camera)
	{
		EntityBuilder builder;
		builder.begin();

		Transform* transform = builder.addTransformComponent();
		transform->m_position = glm::vec2(0, 0);

		ResourceManager* resources = App::instance().resources();
		(void)resources;
		SpriteRenderer* renderer = builder.addSpriteRenderer();

		CameraComponent* cameraComponent = builder.addCameraComponent();
		cameraComponent->m_camera = camera;

		renderer->setTexture(resources->get<TextureResource>(ResourceType::Texture, "effects/crosshair.png"));
		glm::vec2& origin = renderer->getOrigin();
		origin.x = 16.0f; origin.y = 16.0f;
		renderer->setLayer(10);
		renderer->anchor();

		builder.addIdentifier(EntityIdentification::MousePointer);
		Entity* entity = builder.getResults();
		m_scene->registerEntity(entity);
		return entity;
	}
	Entity* EntityFactory::createGladiator(CharacterData& characterData)
	{
		Entity* entity_gladiator;
		EntityBuilder builder;
		builder.begin();

		CharacterComponent* character = builder.addCharacterComponent();
		Gladiator glad;
		glad.m_ownerId = characterData.m_ownerId;
		glad.m_alive = true;
		glad.m_hitpoints = 100;
		*glad.m_position = characterData.m_position;
		glad.m_aimAngle = characterData.m_aimAngle;
		*glad.m_velocity = characterData.m_velocity;
		glad.m_team = characterData.m_team;
		Weapon* weapon = new WeaponGladius();
		glad.m_weapon = weapon;
		glad.setEntityID(characterData.m_id);

		character->m_gladiator = glad;

		ResourceManager* resources = App::instance().resources();

		Transform* transform = builder.addTransformComponent();

		transform->m_position = *glad.m_position;
		Animator* animator = builder.addCharacterAnimator();
		CharacterAnimator& anim = animator->m_animator;

		int skinNumber = characterData.m_team;
		if (skinNumber > 1)
			skinNumber = 0;

		arena::CharacterSkin skin = (CharacterSkin)skinNumber;
		anim.setCharacterSkin(skin);

		std::string tempCrestString;
		std::string tempHelmetString;
		std::string tempTorsoString;
		if (skin == Bronze)
		{
			tempCrestString = "Characters/head/1_Crest4.png";
			tempHelmetString = "Characters/head/1_Helmet.png";
			tempTorsoString = "Characters/body/1_Torso.png";
		}
		else if (skin == Gold)
		{
			tempCrestString = "Characters/head/2_Crest4.png";
			tempHelmetString = "Characters/head/2_Helmet.png";
			tempTorsoString = "Characters/body/2_Torso.png";
		}
		else
		{

			tempCrestString = "Characters/head/1_Crest4.png";
			tempHelmetString = "Characters/head/1_Helmet.png";
			tempTorsoString = "Characters/body/1_Torso.png";
		}

		anim.setStaticContent(
			resources->get<TextureResource>(ResourceType::Texture, tempCrestString),
			resources->get<TextureResource>(ResourceType::Texture, tempHelmetString),
			resources->get<TextureResource>(ResourceType::Texture, tempTorsoString),
			resources->get<SpriterResource>(ResourceType::Spriter, "Characters/Animations/LegAnimations/RunStandJump.scml")->getNewEntityInstance(0),
			resources->get<SpriterResource>(ResourceType::Spriter, "Characters/Animations/DyingAndClimbingAnimations/Dying.scml")->getNewEntityInstance(0),
			resources->get<SpriterResource>(ResourceType::Spriter, "Characters/Animations/ReloadingAndThrowingAnimations/ThrowingGrenade.scml")->getNewEntityInstance(0),
			resources->get<SpriterResource>(ResourceType::Spriter, "Characters/Animations/ReloadingAndThrowingAnimations/Gladius.scml")->getNewEntityInstance(0),
			resources->get<SpriterResource>(ResourceType::Spriter, "Characters/Animations/ReloadingAndThrowingAnimations/Axe.scml")->getNewEntityInstance(0),
			resources->get<SpriterResource>(ResourceType::Spriter, "Characters/Animations/DyingAndClimbingAnimations/Climbing.scml")->getNewEntityInstance(0)

			);
		anim.setWeaponAnimation(WeaponAnimationType::Gladius);

		entity_gladiator = builder.getResults();
		m_scene->registerEntity(entity_gladiator);
		return entity_gladiator;
	
	}


	void EntityFactory::freeEntityId(uint8_t id)
	{
		isIdFree[id] = true;
	}

	uint8_t EntityFactory::getFreeEntityId()
	{
		nextUint8_t(currentFreeId);
		return currentFreeId;
	}

	void EntityFactory::nextUint8_t(uint8_t& current)
	{
		uint8_t old = current;
		current++;

		// search for id till there is a free one left. Will crash if there is none left.
		while (current != old)
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
		printf("No more entity ids left, last id: %d\n", current);
		//assert(false);
	}
}