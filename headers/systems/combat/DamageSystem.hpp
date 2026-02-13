#pragma once

#include "external/entt/fwd.hpp"

namespace Damage
{
	struct Roll;
};
namespace DamageSystem
{
	void take_damage(entt::registry& registry, const entt::entity entity, const Damage::Roll& damage);
};
