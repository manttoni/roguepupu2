#include <string>
#include "entt.hpp"
#include "ECS.hpp"
#include "Color.hpp"
#include "Utils.hpp"
#include "Components.hpp"

namespace ECS
{
	Color get_rarity_color(const std::string& rarity)
	{
		if (rarity == "common")
			return Color(500, 500, 500);
		if (rarity == "uncommon")
			return Color(0, 500, 0);
		if (rarity == "rare")
			return Color(0, 0, 500);
		if (rarity == "epic")
			return Color(500, 0, 250);
		if (rarity == "legendary")
			return Color(900, 750, 150);

		Log::error("Unknown rarity: " + rarity);
	}

	Color get_color(const entt::registry& registry, const entt::entity entity)
	{
		Color c = registry.get<Renderable>(entity).color;
		if (c == Color{})
		{
			if (registry.all_of<Rarity>(entity))
				return get_rarity_color(registry.get<Rarity>(entity).rarity);
			Log::error("Entity color was never defined");
		}
		return c;
	}

	std::string get_colored_name(const entt::registry& registry, const entt::entity entity)
	{
		const auto& color = get_color(registry, entity);
		const auto& name = registry.get<Name>(entity).name;

		std::string colored_name = color.markup() + Utils::capitalize(name) + "{reset}";

		return colored_name;
	}
};
