#include <string>
#include <vector>
#include <map>
#include "entt.hpp"
#include "ECS.hpp"
#include "Cave.hpp"
#include "Color.hpp"
#include "Utils.hpp"
#include "Components.hpp"
#include "systems/StatsSystem.hpp"

namespace ECS
{
	std::string get_category(const entt::registry& registry, const entt::entity entity)
	{
		return registry.get<Category>(entity).category;
	}

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
	std::vector<std::string> get_colored_names(const entt::registry& registry, const std::vector<entt::entity>& items)
	{
		auto player = *registry.view<Player>().begin();
		std::vector<std::string> names;
		for (const auto& item : items)
		{
			std::string name = EquipmentSystem::is_equipped(registry, player, item) ? " * " : "   ";
			name += ECS::get_colored_name(registry, item);
			names.push_back(name);
		}
		return names;
	}
	wchar_t get_glyph(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Renderable>(entity))
		{
			wchar_t glyph = registry.get<Renderable>(entity).glyph;
			if (glyph != L'?')
				return glyph;
		}
		return get_name(registry, entity).front();
	}

	std::string get_name(const entt::registry& registry, const entt::entity entity)
	{
		if (!registry.all_of<Name>(entity))
			return "undefined";
		return registry.get<Name>(entity).name;
	}

	std::string get_colored_name(const entt::registry& registry, const entt::entity entity)
	{
		const auto& color = get_color(registry, entity);
		const auto& name = get_name(registry, entity);
		std::string colored_name = color.markup() + Utils::capitalize(name) + "{reset}";
		return colored_name;
	}

	size_t get_armor_class(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Armor>(entity))
			return registry.get<Armor>(entity).armor_class;
		if (registry.all_of<Stats>(entity))
			return StatsSystem::get_AC(registry, entity);
		return 0;
	}

	Damage get_damage(const entt::registry& registry, const entt::entity entity)
	{
		if (!registry.all_of<Damage>(entity))
			return Damage{};
		return registry.get<Damage>(entity);
	}

	std::string get_subcategory(const entt::registry& registry, const entt::entity entity)
	{
		return registry.get<Subcategory>(entity).subcategory;
	}

	std::string get_proficiency_requirement(const entt::registry& registry, const entt::entity entity)
	{
		const auto& subcategory = registry.get<Subcategory>(entity).subcategory;
		if (subcategory == "weapons")
			return registry.get<Weapon>(entity).proficiency;
		if (subcategory == "armor")
			return registry.get<Armor>(entity).proficiency;
		return "";
	}

	Dice get_versatile_dice(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Weapon>(entity))
			return registry.get<Weapon>(entity).versatile_dice;
		return Dice{};
	}

	std::vector<std::string> get_properties(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Weapon>(entity))
			return registry.get<Weapon>(entity).properties;
		return {};
	}

	double get_normal_range(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Weapon>(entity))
			return registry.get<Weapon>(entity).normal_range;
		return 0;
	}

	double get_long_range(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Weapon>(entity))
			return registry.get<Weapon>(entity).long_range;
		return 0;
	}

	double get_weight(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Weight>(entity))
			return registry.get<Weight>(entity).lb;
		return 0;
	}

	size_t get_value(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Value>(entity))
			return registry.get<Value>(entity).value;
		return 0;
	}

	bool has_weapon_property(const entt::registry& registry, const entt::entity entity, const std::string& property)
	{
		if (!registry.all_of<Weapon>(entity))
			return false;
		const auto& properties = registry.get<Weapon>(entity).properties;
		return std::find(properties.begin(), properties.end(), property) != properties.end();
	}

	Cell* get_cell(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Position>(entity))
			return registry.get<Position>(entity).cell;
		return nullptr;
	}

	std::map<std::string, std::string> get_info(const entt::registry& registry, const entt::entity entity)
	{
		std::map<std::string, std::string> info;

		std::string proficiency = get_proficiency_requirement(registry, entity);
		if (!proficiency.empty())
			info["Proficiency"] = proficiency;

		Damage damage = get_damage(registry, entity);
		if (!damage.type.empty())
		{
			std::string dmg_str = damage.dice.get_string();
			Dice versatile_dice = get_versatile_dice(registry, entity);
			if (!versatile_dice.get_string().empty())
				dmg_str += "/" + versatile_dice.get_string();
			dmg_str += " " + damage.type;
			info["Damage"] = dmg_str;
		}

		if (registry.all_of<Stats>(entity))
		{
			const auto& stats = registry.get<Stats>(entity);
			info["Level"] = std::to_string(stats.level);
			for (const auto& [attribute, value] : stats.attributes)
				info[attribute] = std::to_string(value);
			info["Hitpoints"] = std::to_string(stats.hp) + "/" + std::to_string(stats.max_hp);
			info["AC"] = std::to_string(get_armor_class(registry, entity));
		}

		size_t armor_class = get_armor_class(registry, entity);
		if (armor_class > 0)
			info["AC"] = std::to_string(armor_class);

		std::vector<std::string> properties = get_properties(registry, entity);
		std::string propstring("");
		for (size_t i = 0; i < properties.size(); ++i)
			propstring += properties[i] + (i < properties.size() - 1 ? " | " : "");
		if (!propstring.empty())
			info["Properties"] = propstring;

		double normal_range = get_normal_range(registry, entity);
		double long_range = get_long_range(registry, entity);
		if (normal_range > MELEE_RANGE || long_range > MELEE_RANGE)
			info["Range"] = std::format("{:.1f}", normal_range) + "/" + std::format("{:.1f}", long_range) + " cells";
		else if (normal_range == MELEE_RANGE)
			info["Range"] = "Melee";

		double weight = get_weight(registry, entity);
		if (weight > 0.0)
			info["Weight"] = std::format("{:.1f}", weight) + " lb";

		size_t value = get_value(registry, entity);
		if (value > 0)
			info["Value"] = std::to_string(value) + " gp";

		return info;
	}

	double distance(const entt::registry& registry, const entt::entity a, const entt::entity b)
	{
		Cell* ac = get_cell(registry, a);
		Cell* bc = get_cell(registry, b);

		return ac->get_cave()->distance(*ac, *bc);
	}

	entt::entity get_player(const entt::registry& registry)
	{
		const auto players = registry.view<Player>();
		if (players.empty())
			return entt::null;
		return *players.begin();
	}

	bool is_equippable(const entt::registry& registry, const entt::entity entity)
	{
		const std::vector<std::string> equippable = { "weapons", "armor" };
		return std::find(equippable.begin(), equippable.end(), get_subcategory(registry, entity)) != equippable.end();
	}

	bool can_see(const entt::registry& registry, const entt::entity seer, const entt::entity target)
	{
		if (!registry.all_of<Vision>(seer))
			return false;
		const double vision_range = registry.get<Vision>(seer).range;
		const size_t idx_a = get_cell(registry, seer)->get_idx();
		const size_t idx_b = get_cell(registry, target)->get_idx();
		return get_cell(registry, seer)->get_cave()->has_vision(idx_a, idx_b, vision_range);
	}

	std::vector<entt::entity> get_inventory(const entt::registry& registry, const entt::entity entity)
	{
		if (!registry.all_of<Inventory>(entity))
			return {};
		return registry.get<Inventory>(entity).inventory;
	}
};
