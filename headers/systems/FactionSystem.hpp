#pragma once

#include "entt.hpp"

namespace FactionSystem
{
	bool is_enemy(const entt::registry& registry, const entt::entity a, const entt::entity b);
};
