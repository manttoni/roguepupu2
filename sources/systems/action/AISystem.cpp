#include "components/Components.hpp"
#include "domain/Intent.hpp"
#include "domain/Position.hpp"
#include "external/entt/entt.hpp"
#include "systems/action/AISystem.hpp"
#include "systems/perception/VisionSystem.hpp"
#include "systems/position/MovementSystem.hpp"
#include "systems/state/AlignmentSystem.hpp"
#include "systems/state/EquipmentSystem.hpp"
#include "systems/combat/CombatSystem.hpp"
#include "utils/ECS.hpp"
#include "utils/Random.hpp"

namespace AISystem
{
	bool will_engage_enemy(const entt::registry& registry, Intent& intent)
	{
		// Future idea is to add to visible entities the entities it can remember
		auto visible_entities = VisionSystem::get_visible_entities(registry, intent.actor.entity);
		std::sort(visible_entities.begin(), visible_entities.end(),
				[&](const auto a, const auto b)
				{
				return ECS::distance(registry, a, intent.actor.entity) < ECS::distance(registry, b, intent.actor.entity);
				});

		for (const auto entity : visible_entities)
		{
			if (entity == intent.actor.entity)
				continue;
			if (AlignmentSystem::is_hostile(registry, intent.actor.entity, entity))
			{
				if (CombatSystem::can_attack(registry, intent.actor.entity, entity))
				{
					intent.target.entity = entity;
					intent.type = Intent::Type::Attack; // ActionSystem will expand this to a specific attack type
				}
				else
				{	// Could maybe move this to some other function like will_approach_enemy/entity
					//
					intent.target.position = MovementSystem::get_first_step(registry,
							intent.actor.position,
							registry.get<Position>(entity));
					intent.type = Intent::Type::Move;
					return true;
				}
			}
		}

		return false;
	}

	bool will_wander(const entt::registry& registry, Intent& intent)
	{
		if (!intent.actor.position.is_valid())
			return false;

		const auto& cave = ECS::get_cave(registry, intent.actor.position);
		auto nearby_positions = cave.get_nearby_positions(intent.actor.position, 1.5);
		if (nearby_positions.empty())
			return false;

		std::shuffle(nearby_positions.begin(), nearby_positions.end(), Random::rng());
		for (const auto position : nearby_positions)
		{
			if (MovementSystem::can_move(registry, intent.actor.position, position))
			{
				intent.target.position = position;
				intent.type = Intent::Type::Move;
				return true;
			}
		}
		return false;
	}

	Intent get_npc_intent(const entt::registry& registry, const entt::entity npc)
	{
		if (!registry.all_of<AI>(npc))
			return {.type = Intent::Type::DoNothing};

		const auto& ai = registry.get<AI>(npc);
		Intent intent;
		intent.actor.entity = npc;
		if (registry.all_of<Position>(npc))
			intent.actor.position = registry.get<Position>(npc);

		if (ai.aggressive == true && will_engage_enemy(registry, intent) == true)
			return intent;

		if (ai.idle_wander == true && will_wander(registry, intent) == true)
			return intent;

		// Future plans
		// Swapping equipment
		// Drink potions
		// Flee

		intent.type = Intent::Type::DoNothing;
		return intent;
	}
};
