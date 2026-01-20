#include <cassert>
#include "external/entt/entt.hpp"
#include "systems/position/MovementSystem.hpp"
#include "systems/crafting/GatheringSystem.hpp"
#include "systems/perception/VisionSystem.hpp"
#include "systems/action/AISystem.hpp"
#include "components/Components.hpp"
#include "systems/action/AbilitySystem.hpp"
#include "domain/Cave.hpp"
#include "domain/Cell.hpp"
#include "utils/Log.hpp"
#include "utils/ECS.hpp"


namespace AISystem
{
	bool configure_use_ability(entt::registry& registry, Intent& intent)
	{
		if (!AbilitySystem::has_ability(registry, intent.actor.entity, intent.ability_id))
			Log::error("Entity does not have ability: " + intent.ability_id);

		Ability& ability = registry.get<Abilities>(intent.actor.entity).abilities.at(intent.ability_id);
		if (AbilitySystem::on_cooldown(registry, ability))
			return false;

		switch (ability.effect.type)
		{
			case Effect::Type::CreateEntity:
				if (ability.range < 1.0)
					intent.target.position = intent.actor.position;
				else
					Log::error("Logic not implemented for bigger range");
				break;
			default:
				Log::error("Unhandled ability effect type");
		}

		return true;
	}

	bool configure_gather(const entt::registry& registry, Intent& intent)
	{
		auto visible_positions = VisionSystem::get_visible_positions(registry, intent.actor.entity);
		std::sort(visible_positions.begin(), visible_positions.end(), [&](const Position& a, const Position& b){
				const auto a_distance = ECS::distance(registry, intent.actor.position, a);
				const auto b_distance = ECS::distance(registry, intent.actor.position, b);
				return a_distance < b_distance;
				});

		for (const auto visible_pos : visible_positions)
		{
			for (const auto entity : ECS::get_entities(registry, visible_pos))
			{
				if (!registry.all_of<Gatherable>(entity) || !GatheringSystem::has_tool(registry, intent.actor.entity, entity))
					continue;
				if (ECS::distance(registry, intent.actor.position, visible_pos) > 1.5)
				{
					intent.type = Intent::Type::Move;
					intent.target.position = MovementSystem::get_first_step(registry, intent.actor.position, visible_pos);
					return true;
				}
				intent.target.entity = entity;
				return true;
			}
		}
		return false;
	}

	Intent get_npc_intent(const entt::registry& registry, const entt::entity npc)
	{
		if (!registry.all_of<AI, Position>(npc))
			return {.type = Intent::Type::DoNothing};

		return {.type = Intent::Type::DoNothing};
	}
};
