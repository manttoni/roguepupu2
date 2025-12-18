#include "systems/DamageSystem.hpp"
#include "systems/VisualEffectSystem.hpp"
#include "GameLogger.hpp"
#include "entt.hpp"
#include "Components.hpp"
#include "ECS.hpp"

namespace DamageSystem
{
	void kill(entt::registry& registry, const entt::entity entity)
	{
		registry.ctx().get<GameLogger>().log(ECS::get_colored_name(registry, entity) + " dies");

		/*// Change to corpse/remains glyph
		if (!registry.all_of<Glyph>(entity))
			registry.emplace<Glyph>(entity);
		registry.get<Glyph>(entity).glyph = L'X';*/

		// Add red/bloody background color to cell
		Cell* cell = ECS::get_cell(registry, entity);
		cell->set_bg(Color(250, 0, 0));

		// Remove movement blocking component
		if (registry.all_of<Solid>(entity))
			registry.remove<Solid>(entity);

		// Remove Faction
		if (registry.all_of<Faction>(entity))
			registry.remove<Faction>(entity);

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
