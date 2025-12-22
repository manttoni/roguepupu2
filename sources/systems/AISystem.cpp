#include "systems/AISystem.hpp"
#include "Intent.hpp"
#include "entt.hpp"
#include "Components.hpp"
#include "ECS.hpp"
#include "GameLogger.hpp"
#include "Utils.hpp"

namespace AISystem
{
	Intent get_npc_intent(const entt::registry& registry, const entt::entity npc)
	{
		const auto& ai = registry.get<AI>(npc);
		for (const auto& intent : ai.intentions)
		{
			Intent result = intent;
			switch (intent.type)
			{
				case Intent::Type::UseAbility: {
					const Ability& ability = registry.get<Abilities>(npc).abilities.at(intent.ability_id);
					if (ability.on_cooldown(registry)) continue;
					if (ability.get_target() == "self")
					{
						result.target_cell = ECS::get_cell(registry, npc);
						result.target = npc;
					}
					return result;
				}
				default:
					return {.type = Intent::Type::DoNothing};
			}
		}
		return {.type = Intent::Type::DoNothing};
	}
};
