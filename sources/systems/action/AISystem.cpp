#include "components/Components.hpp"
#include "domain/Intent.hpp"
#include "domain/Position.hpp"
#include "external/entt/entt.hpp"
#include "systems/action/AISystem.hpp"
#include "systems/perception/VisionSystem.hpp"
#include "systems/position/MovementSystem.hpp"
#include "systems/state/AlignmentSystem.hpp"
#include "systems/state/EquipmentSystem.hpp"
#include "utils/ECS.hpp"
#include "utils/Random.hpp"

namespace AISystem
{
	bool will_engage_enemy(const entt::registry& registry, Intent& intent)
	{
		if (!intent.actor.position.is_valid())
			return false;

		auto visible_entities = VisionSystem::get_visible_entities(registry, intent.actor.entity);
		std::sort(visible_entities.begin(), visible_entities.end(),
				[&](const auto a, const auto b)
				{
				return ECS::distance(registry, a, intent.actor.entity) < ECS::distance(registry, b, intent.actor.entity);
				});
		for (const auto entity : visible_entities)
		{
			if (!registry.any_of<Alignment>(entity))
				continue;
			if (AlignmentSystem::is_hostile(registry, intent.actor.entity, entity))
			{
				// npc will attack if it has at least one weapon with enough range
				const auto distance = ECS::distance(registry, intent.actor.entity, entity);
				if (ECS::get_attack_range(registry, intent.actor.entity).contains(distance))
				{
					intent.target.entity = entity;
					intent.type = Intent::Type::Attack;
					return true;
				}

				// npc might want to swap loadout, if it has a weapon with more suitable range
				if (registry.all_of<EquipmentSlots>(intent.actor.entity))
				{
					const auto& slots = registry.get<EquipmentSlots>(intent.actor.entity);
					const auto& other_loadout = slots.get_other_loadout();
					if ((other_loadout.main_hand != entt::null && ECS::get_attack_range(registry, other_loadout.main_hand).contains(distance)) ||
						(other_loadout.off_hand != entt::null && ECS::get_attack_range(registry, other_loadout.off_hand).contains(distance)))
					{
						intent.type = Intent::Type::SwapLoadout;
						return true;
					}
				}

				// no attack was suitable, then approach
				intent.target.position = MovementSystem::get_first_step(
						registry,
						intent.actor.position,
						registry.get<Position>(entity));
				intent.type = Intent::Type::Move;
				return true;
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

		intent.type = Intent::Type::DoNothing;
		return intent;
	}
};
