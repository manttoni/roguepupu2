#pragma once

#include "entt.hpp"
#include "Event.hpp"

namespace ConditionSystem
{
	bool is_true(const entt::registry& registry, const Conditions& conditions, const entt::entity entity);
};
