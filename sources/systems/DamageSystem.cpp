#include "systems/DamageSystem.hpp"
#include "systems/VisualEffectSystem.hpp"
#include "GameLogger.hpp"
#include "GameState.hpp"
#include "entt.hpp"
#include "Components.hpp"
#include "ECS.hpp"

namespace DamageSystem
{
	void kill(entt::registry& registry, const entt::entity entity)
	{
		registry.ctx().get<GameLogger>().log(ECS::get_colored_name(registry, entity) + " dies");
		if (entity == ECS::get_player(registry))
			registry.ctx().get<GameState>().running = false;

		registry.emplace_or_replace<Glyph>(entity, L'x');
		registry.emplace_or_replace<Dead>(entity, registry.ctx().get<GameState>().turn_number);
		if (registry.all_of<Solid>(entity))
			registry.remove<Solid>(entity);

		registry.get<Name>(entity).name += " (corpse)";

	}

	void take_damage(entt::registry& registry, const entt::entity entity, const int damage)
	{
		assert(damage >= 0);
		auto& resources = registry.get<Resources>(entity);
		resources.health -= damage;
		registry.ctx().get<GameLogger>().log(ECS::get_colored_name(registry, entity) + " takes {500,0,0}" + std::to_string(damage) + "{reset} physical damage");
		VisualEffectSystem::damage_flash(registry, entity);
		if (resources.health <= 0)
			kill(registry, entity);
	}
};
