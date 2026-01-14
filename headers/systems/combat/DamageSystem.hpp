#pragma once

#include "entt.hpp"  // for entity, registry

namespace DamageSystem
{
	void take_damage(entt::registry& registry, const entt::entity entity, const int damage);
};
