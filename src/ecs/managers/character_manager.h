#pragma once

#include "../../mem/pool_allocator.h"
#include "component_manager.h"
#include "../character_component.h"
#include "../../singleton.h"
#include "../common/arena/arena_packet.h"
#include "../entity_factory.h"

namespace arena
{
	class CharacterManager final : public ComponentManager<CharacterComponent>
	{
	public:
		MAKE_SINGLETON(CharacterManager)
		void applyUpdatePacket(GameUpdatePacket* packet);
		void setFactory(EntityFactory* factory);
		void characterShoot(BulletData &data);
		float rotateCharacterAnimationAim(uint8_t playerId, glm::vec2 aimTarget);
		void applyDamageToCharacter(GameDamagePlayerPacket* packet);
		void killCharacter(GameKillPlayerPacket* packet);
		void respawnCharacter(GameRespawnPlayerPacket* packet);
		unsigned getCharacterAmount();
		CharacterComponent* getCharacterByPlayerId(uint8_t playerId);
		virtual void onUpdate(const GameTime& gameTime) final override;
	protected:
	private:
		EntityFactory* m_factory;
	};
}