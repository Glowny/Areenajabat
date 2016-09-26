#include "character_manager.h"
#include "../transform.h"
#include "../entity.h"
#include "../animator.h"
namespace arena
{
	CharacterManager::CharacterManager() : ComponentManager()
	{
	}
	void CharacterManager::onUpdate(const GameTime& gameTime)
	{
		gameTime;
		for(auto it = begin(); it != end(); ++it)
		{
			Transform* transform = (Transform*)(*it)->owner()->first(TYPEOF(Transform));
			glm::vec2 position = *(*it)->m_gladiator.m_position;
			transform->m_position = glm::vec2(position.x - 20.0f, position.y - 60.0f);
			Animator* animator = (Animator* const)((*it)->owner()->first(TYPEOF(Animator)));
			animator->rotateAimTo((*it)->m_gladiator.m_aimAngle);
		}
	}

	void CharacterManager::setFactory(EntityFactory* factory)
	{
		m_factory = factory;
	}

	void CharacterManager::characterShoot(BulletData &data)
	{
		CharacterComponent* chara = getCharacterByPlayerId(data.m_ownerId);
		Animator* animator = (Animator* const)(chara->owner()->first(TYPEOF(Animator)));
		animator->m_animator.setRecoil(true);
		m_factory->createProjectile(data);
	}

	// Calculates aim angle between a character and a point.
	// Note: this is only used on player's character.
	float CharacterManager::rotateCharacterAnimationAim(uint8_t playerId, glm::vec2 aimTarget)
	{
		CharacterComponent* chara = getCharacterByPlayerId(playerId);
		Animator* animator = (Animator* const)(chara->owner()->first(TYPEOF(Animator)));
		Transform* playerTransform = (Transform* const)(chara->owner()->first(TYPEOF(Transform)));
	
		float xOffset = animator->m_animator.m_shoulderPoint.x;
		float yOffset = animator->m_animator.m_shoulderPoint.y;
		glm::vec2 mouseLoc = aimTarget;

		glm::vec2 m_weaponRotationPoint = glm::vec2(playerTransform->m_position.x + 9 + xOffset, playerTransform->m_position.y + 14 - yOffset);
		glm::vec2 dir(mouseLoc - m_weaponRotationPoint);
		float a = glm::atan(dir.y, dir.x);
	
		// Update own gladiator aim
		chara->m_gladiator.m_aimAngle = a;
		return a;
	}

	unsigned CharacterManager::getCharacterAmount()
	{
		unsigned amount = 0;
		for (auto it = begin(); it != end(); ++it)
		{
			amount++;
		}
		return amount;
	}

	void CharacterManager::applyUpdatePacket(GameUpdatePacket* packet)
	{
		for (int32_t i = 0; i < packet->m_playerAmount; i++)
		{
			uint8_t playerId = packet->m_characterArray[i].m_ownerId;
			CharacterComponent* character = getCharacterByPlayerId(playerId);
			if (character == nullptr)
			{
				printf("Gladiator of id %d not found in applyUpdatePacket\n", playerId);
				continue;
			}
			Entity* entity = character->owner();
			Animator* animator = (Animator* const)(entity->first(TYPEOF(Animator)));
			Gladiator& gladiator = character->m_gladiator;
			
			*gladiator.m_position = packet->m_characterArray[i].m_position;
			*gladiator.m_velocity = packet->m_characterArray[i].m_velocity;
			gladiator.m_aimAngle = packet->m_characterArray[i].m_aimAngle;
			// TODO: Player team should not be set all the time.
			gladiator.m_team = packet->m_characterArray[i].m_team;

			if (packet->m_characterArray[i].m_reloading)
			{
				animator->m_animator.playReloadAnimation();
				bool flip = animator->m_animator.getUpperBodyDirection();
				glm::vec2 createPos = *gladiator.m_position;
				createPos = glm::vec2(createPos.x + flip * 70 - 60, createPos.y - 24.0f);
				m_factory->createMagazine(createPos, *gladiator.m_velocity + glm::vec2(2.0f, 2.0f), flip);
			}
			if (packet->m_characterArray[i].m_throwing)
			{
				animator->m_animator.playThrowAnimation(0);
			}
			if (packet->m_characterArray[i].m_climbing != 0)
			{
				animator->m_animator.playClimbAnimation(packet->m_characterArray[i].m_climbing);
			}
			else
			{
				animator->m_animator.endClimbAnimation();
			}

			// Set player legs move according to x-velocity.
			glm::vec2 moveSpeed = packet->m_characterArray[i].m_velocity;
			// Max movement speed is 300.
			if (moveSpeed.x < -15.0f)
			{
				animator->m_animator.setFlipX(0);
				animator->m_animator.startRunningAnimation(fabs(moveSpeed.x / 300.0f));
			}
			else if (moveSpeed.x > 15.0f)
			{
				animator->m_animator.setFlipX(1);
				animator->m_animator.startRunningAnimation(fabs(moveSpeed.x / 300.0f));
			}
			else
			{
				animator->m_animator.stopRunningAnimation();

			}

			// If gladiator is climbing a ladder, decide if still, going up or going down animation is played.
			if (animator->m_animator.isClimbing())
			{
				if (moveSpeed.y < -50.0f)
					animator->m_animator.continueClimbAnimation(1, moveSpeed.y);

				else if (moveSpeed.y > 50.0f)
				{
					animator->m_animator.continueClimbAnimation(-1, moveSpeed.y);
				}
				else
				{
					animator->m_animator.pauseClimbAnimation();
				}
			}
		}
	}

	void CharacterManager::applyDamageToCharacter(GameDamagePlayerPacket* packet)
	{
		CharacterComponent* character = getCharacterByPlayerId(packet->m_targetID);
		if (character == nullptr)
		{
			printf("Gladiator of id %d not found in applyDamageToCharacter\n", packet->m_targetID);
			return;
		}
		Animator* animator = (Animator* const)(character->owner()->first(TYPEOF(Animator)));
		character->m_gladiator.m_hitpoints -= int32(packet->m_damageAmount);
		if (character->m_gladiator.m_hitpoints <= 0)
		{
			animator->m_animator.playDeathAnimation(packet->m_hitDirection, packet->m_hitPosition.y - character->m_gladiator.m_position->y);
			m_factory->createMiniBomb(packet->m_targetID, 1.5f);
		}
		m_factory->createBulletHitBlood(packet->m_hitDirection, glm::vec2(character->m_gladiator.m_position->x, packet->m_hitPosition.y));

	}
	void CharacterManager::killCharacter(GameKillPlayerPacket* packet)
	{
		CharacterComponent* character = getCharacterByPlayerId(packet->m_playerID);
		if (character == nullptr)
		{
			printf("Gladiator of id %d not found in killCharacter\n", packet->m_playerID);
			return;
		}
		character->m_gladiator.m_hitpoints = 0;
		character->m_gladiator.m_alive = false;
	}
	void CharacterManager::respawnCharacter(GameRespawnPlayerPacket* packet)
	{
		CharacterComponent* character = getCharacterByPlayerId(packet->m_playerID);
		if (character == nullptr)
		{
			printf("Gladiator of id %d not found in respawnCharacter\n", packet->m_playerID);
			return;
		}
		Animator* animator = (Animator* const)(character->owner()->first(TYPEOF(Animator)));
		character->m_gladiator.m_hitpoints = 100;
		character->m_gladiator.m_alive = true;
		animator->m_animator.resetAnimation();
		animator->m_animator.hide = false;
	}


	CharacterComponent* CharacterManager::getCharacterByPlayerId(uint8_t playerId)
	{
		for (auto it = begin(); it != end(); ++it)
		{
			if ((*it)->m_gladiator.m_ownerId == playerId)
				return (*it);
		}
		// No match.
		return nullptr;
	}
}