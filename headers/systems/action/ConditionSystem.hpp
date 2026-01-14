#pragma once

#include <cstdint>
#include "external/entt/fwd.hpp"

struct Conditions;

namespace ConditionSystem
{
	bool is_true(const entt::registry& registry, const Conditions& conditions, const entt::entity entity);
};
