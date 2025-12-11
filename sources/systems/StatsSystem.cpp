#include "systems/StatsSystem.hpp"
#include "Components.hpp"
#include "ECS.hpp"

namespace StatsSystem
{
	int get_modifier(const size_t attribute)
	{
		return (attribute - 10) / 2;
	}

	size_t get_AC(const entt::registry& registry, const entt::entity entity)
	{
		if (!registry.all_of<Stats>(entity))
			return 0;

		const auto& stats = registry.get<Stats>(entity);
		const size_t dexterity = stats.attributes.at("dexterity");
		const int dexmod = get_modifier(dexterity);
		size_t AC = 10;
		if (registry.all_of<Equipment>(entity))
		{
			const auto& equipment = registry.get<Equipment>(entity);
			const size_t armor_AC = ECS::get_armor_class(registry, equipment.armor);
			AC = std::max(AC, armor_AC);
		}
		return AC + dexmod;
	}
};
