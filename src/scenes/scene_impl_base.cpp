#include "scene_impl_base.h"

namespace arena
{
	const String& SceneImplBase::name() const
	{
	}

	SceneImplBase::SceneImplBase(const String& name) : m_name(name)
	{
	}

	void SceneImplBase::registerEntity(Entity* const entity)
	{
	}
	void SceneImplBase::unregisterEntity(Entity* const entity)
	{
	}

	EntityIterator SceneImplBase::entititesBegin()
	{
	}
	EntityIterator SceneImplBase::entititesEnd()
	{
	}

	Entity* const SceneImplBase::find(Predicate<Entity* const> pred)
	{
	}
}