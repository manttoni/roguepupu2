#pragma once

#include <cstddef>
#include "entt.hpp"

namespace StatsSystem
{
	int get_modifier(const size_t attribute);
	size_t get_AC(const entt::registry& registry, const entt::entity entity);
};
