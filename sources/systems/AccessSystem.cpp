#include "systems/AccessSystem.hpp"
#include "entt.hpp"

namespace AccessSystem
{
	void unlock(entt::registry& registry, const entt::entity actor, const entt::entity object)
	{
		(void) registry; (void) actor; (void) object;
	}

	void open(entt::registry& registry, const entt::entity actor, const entt::entity object)
	{
		(void) registry; (void) actor; (void) object;
	}

	bool is_locked(const entt::registry& registry, const entt::entity entity)
	{
		(void) registry; (void) entity;
		return false;
	}

	bool is_closed(const entt::registry& registry, const entt::entity entity)
	{
		(void) registry; (void) entity;
		return false;
	}
};
