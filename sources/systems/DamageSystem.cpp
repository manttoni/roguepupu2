#include "systems/DamageSystem.hpp"
#include "systems/VisualEffectSystem.hpp"
#include "GameLogger.hpp"
#include "entt.hpp"
#include "Components.hpp"
#include "ECS.hpp"

namespace DamageSystem
{
	void take_damage(entt::registry& registry, const entt::entity entity, const int damage)
	{
		assert(damage >= 0);
		auto& resources = registry.get<Resources>(entity);
		resources.health -= damage;
		registry.ctx().get<GameLogger>().log(ECS::get_colored_name(registry, entity) + " takes {500,0,0}" + std::to_string(damage) + "{reset} physical damage");
		VisualEffectSystem::damage_flash(registry, entity);
		if (resources.health <= 0)
			registry.ctx().get<GameLogger>().log(ECS::get_colored_name(registry, entity) + " dies");
	}
};
