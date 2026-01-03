#include <cassert>
#include "systems/AISystem.hpp"
#include "Event.hpp"
#include "entt.hpp"
#include "Components.hpp"
#include "ECS.hpp"
#include "GameLogger.hpp"
#include "Utils.hpp"
#include "systems/AbilitySystem.hpp"

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

	Intent get_npc_intent(entt::registry& registry, const entt::entity npc)
	{
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
				default:
					break;
			}
		}
		return {.type = Intent::Type::DoNothing};
	}
};
