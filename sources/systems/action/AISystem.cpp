#include <cassert>
#include "systems/position/PositionSystem.hpp"
#include "systems/position/MovementSystem.hpp"
#include "systems/crafting/GatheringSystem.hpp"
#include "systems/perception/VisionSystem.hpp"
#include "systems/action/AISystem.hpp"
#include "components/Components.hpp"
#include "systems/action/AbilitySystem.hpp"
#include "domain/Cave.hpp"
#include "domain/Cell.hpp"

namespace AISystem
{
	bool configure_use_ability(entt::registry& registry, const entt::entity npc, Intent& intent)
	{
		Ability& ability = registry.get<Abilities>(npc).abilities.at(intent.ability_id);
		assert(ability.target.type != Target::Type::None);
		intent.target = ability.target;
		if (AbilitySystem::on_cooldown(registry, ability))
			return false;
		if (intent.target.type == Target::Type::Cell && intent.target.range == 0)
			intent.target.position = registry.get<Position>(npc);
		else if (intent.target.type == Target::Type::Self)
			intent.target.entity = npc;
		assert(intent.target.type != Target::Type::None);
		return true;
	}

	bool configure_gather(const entt::registry& registry, const entt::entity npc, Intent& intent)
	{
		const auto& npc_pos = registry.get<Position>(npc);
		const auto cave_idx = npc_pos.cave_idx;
		auto visible_cells = VisionSystem::get_visible_cells(registry, npc);
		std::sort(visible_cells.begin(), visible_cells.end(), [&](const size_t a, const size_t b){
				const auto a_distance = PositionSystem::distance(registry, npc_pos, {a, cave_idx});
				const auto b_distance = PositionSystem::distance(registry, npc_pos, {b, cave_idx});
				return a_distance < b_distance;
				});

		for (const auto idx : visible_cells)
		{
			const auto visible_position = Position{idx, cave_idx};
			const auto& entities = ECS::get_entities(registry, visible_position);
			for (const auto entity : entities)
			{
				if (!registry.all_of<Gatherable>(entity) || !GatheringSystem::has_tool(registry, npc, entity))
					continue;
				if (PositionSystem::distance(registry, npc_pos, visible_position) > MELEE_RANGE)
				{
					intent.type = Intent::Type::Move;
					intent.target.position = MovementSystem::get_first_step(registry, npc_pos, visible_position);
					return true;
				}
				intent.target.entity = entity;
				return true;
			}
		}
		return false;
	}

	Intent get_npc_intent(entt::registry& registry, const entt::entity npc)
	{
		if (!registry.all_of<AI>(npc))
			return {.type = Intent::Type::DoNothing};
		const auto& ai = registry.get<AI>(npc);
		for (const auto& intent : ai.intentions)
		{
			Intent result = intent;
			switch (intent.type)
			{
				case Intent::Type::UseAbility:
					if (configure_use_ability(registry, npc, result) == true)
						return result;
					continue;
				case Intent::Type::Hide:
					if (!registry.all_of<Hidden>(npc))
						return {.type = Intent::Type::Hide};
					continue;
				case Intent::Type::Gather:
					if (configure_gather(registry, npc, result) == true)
						return result;
					continue;
				default:
					break;
			}
		}
		return {.type = Intent::Type::DoNothing};
	}
};
