#pragma once

#include "external/entt/fwd.hpp"

struct Damage;
namespace DamageSystem
{
	void take_damage(entt::registry& registry, const entt::entity entity, const Damage& damage);
};
