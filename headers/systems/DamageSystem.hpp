#pragma once

#include <vector>
#include "entt.hpp"
#include "Components.hpp"

namespace DamageSystem
{
	void take_damage(entt::registry& registry, const entt::entity entity, const int damage);
};
