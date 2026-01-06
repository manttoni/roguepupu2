#include <cassert>
#include "systems/MovementSystem.hpp"
#include "systems/GatheringSystem.hpp"
#include "systems/VisionSystem.hpp"
#include "systems/AISystem.hpp"
#include "Event.hpp"
#include "entt.hpp"
#include "Components.hpp"
#include "ECS.hpp"
#include "GameLogger.hpp"
#include "Utils.hpp"
#include "systems/AbilitySystem.hpp"
#include "Cave.hpp"
#include "Cell.hpp"

namespace AISystem
{
	bool configure_use_ability(entt::registry& registry, const entt::entity npc, Intent& intent)
	{
		Ability& ability = registry.get<Abilities>(npc).abilities.at(intent.ability_id);
		assert(ability.target.type != Target::Type::None);
		intent.ability = &ability;
		intent.target = ability.target;
		if (AbilitySystem::on_cooldown(registry, ability))
			return false;
		if (intent.target.type == Target::Type::Cell && intent.target.range == 0)
			intent.target.cell = ECS::get_cell(registry, npc);
		else if (intent.target.type == Target::Type::Self)
			intent.target.entity = npc;
		assert(intent.target.type != Target::Type::None);
		return true;
	}

	bool configure_gather(const entt::registry& registry, const entt::entity npc, Intent& intent)
	{
		Cell* cell = ECS::get_cell(registry, npc);
		Cave* cave = cell->get_cave();
		const auto npc_idx = cell->get_idx();

		auto visible_cells = VisionSystem::get_visible_cells(registry, npc);
		std::sort(visible_cells.begin(), visible_cells.end(), [&](const size_t a, const size_t b){
				return cave->distance(npc_idx, a) < cave->distance(npc_idx, b);
				});

		for (const auto idx : visible_cells)
		{
			const auto& entities = cave->get_cell(idx).get_entities();
			for (const auto entity : entities)
			{
				if (!registry.all_of<Gatherable>(entity) || !GatheringSystem::has_tool(registry, npc, entity))
					continue;
				if (cave->distance(npc_idx, idx) > MELEE_RANGE)
				{
					intent.type = Intent::Type::Move;
					intent.target.cell = MovementSystem::get_first_step(registry, npc, cave->get_cell(idx));
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
