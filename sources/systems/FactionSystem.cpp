#include "systems/FactionSystem.hpp"
#include "entt.hpp"
#include "Components.hpp"

namespace FactionSystem
{
	bool is_enemy(const entt::registry& registry, const entt::entity a, const entt::entity b)
	{
		if (!registry.all_of<Faction>(a) || !registry.all_of<Faction>(b))
			return false;

		const auto a_f = registry.get<Faction>(a).faction;
		const auto b_f = registry.get<Faction>(b).faction;

		if (a_f == b_f)
			return false;

		if (a_f == "player" || b_f == "player")
		{
			if (a_f == "spiders" || b_f == "spiders")
				return false;
		}

		return true;
	}
};
