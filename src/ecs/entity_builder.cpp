#include "entity_builder.h"

#include "entity.h"
#include "transform.h"
#include "sprite_renderer.h"
#include "..\res\texture_resource.h"
#include "managers\transform_manager.h"
#include "managers\sprite_manager.h"

#include <cassert>

namespace arena
{
	//void EntityBuilder::begin()
	//{
	//	m_entity = Entity::create();
	//}

	//Transform* const EntityBuilder::addTransformComponent()
	//{
	//	assert(m_entity != nullptr);

	//	Transform* const transform = TransformManager::instance().create();

	//	m_entity->add(transform);

	//	return transform;
	//}
	//SpriteRenderer* const EntityBuilder::addSpriteRenderer()
	//{
	//	assert(m_entity != nullptr);

	//	SpriteRenderer* const renderer = SpriteManager::instance().create();

	//	m_entity->add(renderer);

	//	return renderer;
	//}

	//void EntityBuilder::addTag(const String& tag)
	//{
	//	assert(m_entity != nullptr);

	//	m_entity->setTags(m_entity->getTags() + "|" + tag);
	//}
	//void EntityBuilder::setTags(const String& tags)
	//{
	//	assert(m_entity != nullptr);

	//	m_entity->setTags(tags);
	//}

	//Entity* EntityBuilder::getResults()
	//{
	//	Entity* results = m_entity;
	//	
	//	m_entity = nullptr;

	//	return results;
	//}
}