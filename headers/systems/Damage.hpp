#pragma once

#include "external/entt/fwd.hpp"
#include "domain/Enum.hpp"


                namespace System::Damage

{
	void take_damage(entt::registry& registry, const entt::entity entity, const size_t amount, const Domain::Enum::DamageType damage_type);
};
