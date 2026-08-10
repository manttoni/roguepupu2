#pragma once

#include "external/entt/fwd.hpp"
#include "domain/Enum.hpp"

namespace DamageSystem
{
	void take_damage(entt::registry& registry, const entt::entity entity, const size_t amount, const Enum::DamageType damage_type);
};
