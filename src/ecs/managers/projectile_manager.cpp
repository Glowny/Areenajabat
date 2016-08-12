#include "projectile_manager.h"
#include "../transform.h"
#include "../entity.h"
namespace arena
{
	ProjectileManager::ProjectileManager() : ComponentManager()
	{
	}
	void ProjectileManager::onUpdate(const GameTime& gameTime)
	{
		gameTime;
		for(auto it = begin(); it != end(); ++it)
		{
			// TODO: Consider setting transform position pointer to pointer of bullet position.
			// Set transform component position to the position of bullet.
			Transform* bulletTransform = (Transform* const)(*it)->owner()->first(TYPEOF(Transform));	
			bulletTransform->m_position = *(*it)->bullet->m_position;

		}
	}
}