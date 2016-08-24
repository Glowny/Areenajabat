#include "physics_manager.h"
#include "../transform.h"
#include "../sprite_renderer.h"
#include "../entity.h"
namespace arena
{
	PhysicsManager::PhysicsManager() : ComponentManager()
	{
	}
	void PhysicsManager::onUpdate(const GameTime& gameTime)
	{
		gameTime;
		for(auto it = begin(); it != end(); ++it)
		{
			PhysicsComponent* physicsComponent = (PhysicsComponent *)(*it)->owner()->first(TYPEOF(PhysicsComponent));
			SpriteRenderer* renderer = (SpriteRenderer*)(*it)->owner()->first(TYPEOF(SpriteRenderer));
			if (renderer == nullptr)
				return;
			float rotation = m_physics->getEntityRotation(physicsComponent->m_physicsId);
			renderer->setRotation(rotation);
			
		}
	}
}