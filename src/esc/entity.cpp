#include "entity.h"
#include "component.h"

namespace arena
{
	void Entity::destroy() {
		// TODO: release all components.
	}

	void Entity::add(Component* const component) {
		m_components.push_back(component);
	}
	void Entity::remove(Component* const component) {
		m_components.erase(std::find(m_components.begin(), m_components.end(), component));
	}

	const String& Entity::getTags() const {
		return m_tags;
	}
	void Entity::setTags(const String& tags) {
		m_tags = tags;
	}

	ComponentIterator Entity::begin() {
		return m_components.begin();
	}
	ComponentIterator Entity::end() {
		return m_components.end();
	}
}