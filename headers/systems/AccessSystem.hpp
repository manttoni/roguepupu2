#pragma once

#include "entt.hpp"

namespace AccessSystem
{
	void unlock(entt::registry& registry, const entt::entity actor, const entt::entity object);
	void open(entt::registry& registry, const entt::entity actor, const entt::entity object);
	bool is_locked(const entt::registry& registry, const entt::entity entity);
	bool is_closed(const entt::registry& registry, const entt::entity entity);
};
