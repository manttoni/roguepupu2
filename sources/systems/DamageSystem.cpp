#include "systems/DamageSystem.hpp"

namespace DamageSystem
{
	void take_damage(entt::registry& registry, const entt::entity entity, const int damage)
	{
		auto& resources = registry.get<Resources>(entity);
		resources.health -= damage;
	}
};
