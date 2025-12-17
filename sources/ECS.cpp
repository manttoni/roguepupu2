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
#include "systems/EquipmentSystem.hpp"
#include "systems/CombatSystem.hpp"
#include "systems/InventorySystem.hpp"

namespace ECS
{
	Color get_color(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<FGColor>(entity))
			return registry.get<FGColor>(entity).color;
		return Color(500, 500, 500);
	}

	std::string get_colored_name(const entt::registry& registry, const entt::entity entity)
	{
		const auto& color = get_color(registry, entity);
		const auto& name = registry.get<Name>(entity).name;
		std::string colored_name = color.markup() + Utils::capitalize(name) + "{reset}";
		return colored_name;
	}

	std::vector<std::string> get_colored_names(const entt::registry& registry, const std::vector<entt::entity>& items)
	{
		auto player = get_player(registry);
		std::vector<std::string> names;
		for (const auto& item : items)
		{
			std::string name = EquipmentSystem::is_equipped(registry, player, item) ? " * " : "   ";
			name += get_colored_name(registry, item);
			names.push_back(name);
		}
		return names;
	}

	wchar_t get_glyph(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Glyph>(entity))
			return registry.get<Glyph>(entity).glyph;
		return registry.get<Name>(entity).name.front();
	}

	Cell* get_cell(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Position>(entity))
			return registry.get<Position>(entity).cell;
		return nullptr;
	}

	int get_damage_min(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Damage>(entity))
			return registry.get<Damage>(entity).min;
		if (registry.all_of<Equipment>(entity))
		{
			const auto& equipment = registry.get<Equipment>(entity);
			double total_min = static_cast<double>(get_damage_min(registry, equipment.right_hand));
			if (EquipmentSystem::is_dual_wielding(registry, entity))
			{
				total_min += static_cast<double>(get_damage_min(registry, equipment.left_hand));
				total_min *= 0.75;
			}
			return static_cast<int>(total_min);
		}
		return 0;
	}

	int get_damage_max(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Damage>(entity))
			return registry.get<Damage>(entity).max;
		if (registry.all_of<Equipment>(entity))
		{
			const auto& equipment = registry.get<Equipment>(entity);
			double total_max = static_cast<double>(get_damage_max(registry, equipment.right_hand));
			if (EquipmentSystem::is_dual_wielding(registry, entity))
			{
				total_max += static_cast<double>(get_damage_max(registry, equipment.left_hand));
				total_max *= 0.75;
			}
			return static_cast<int>(total_max);
		}
		return 0;
	}

	int get_armor_penetration(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<ArmorPenetration>(entity))
			return registry.get<ArmorPenetration>(entity).armor_penetration;
		return 0;
	}

	int get_armor(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Armor>(entity))
			return registry.get<Armor>(entity).armor;
		if (registry.all_of<Equipment, Attributes>(entity))
		{
			const auto& equipment = registry.get<Equipment>(entity);
			return get_strength(registry, entity) * get_armor(registry, equipment.armor);
		}
		return 0;
	}

	int get_accuracy(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Accuracy>(entity))
			return registry.get<Accuracy>(entity).accuracy;
		return 0;
	}

	int get_evasion(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Evasion>(entity))
			return registry.get<Evasion>(entity).evasion;
		return 0;
	}

	int get_barrier(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Barrier>(entity))
			return registry.get<Barrier>(entity).barrier;
		return 0;
	}

	int get_power(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Power>(entity))
			return registry.get<Power>(entity).power;
		return 0;
	}

	double get_crit_chance(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<CritChance>(entity))
			return registry.get<CritChance>(entity).crit_chance;
		return 0;
	}

	double get_crit_multiplier(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<CritMultiplier>(entity))
			return registry.get<CritMultiplier>(entity).crit_multiplier;
		return 0;
	}

	int get_strength(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Attributes>(entity))
			return registry.get<Attributes>(entity).strength;
		return 0;
	}
	int get_dexterity(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Attributes>(entity))
			return registry.get<Attributes>(entity).dexterity;
		return 0;
	}
	int get_intelligence(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Attributes>(entity))
			return registry.get<Attributes>(entity).intelligence;
		return 0;
	}

	int get_health_max(const entt::registry& registry, const entt::entity entity)
	{
		return get_strength(registry, entity);
	}
	int get_fatigue_max(const entt::registry& registry, const entt::entity entity)
	{
		return get_dexterity(registry, entity);
	}
	int get_mana_max(const entt::registry& registry, const entt::entity entity)
	{
		return get_intelligence(registry, entity);
	}

	double get_weight(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Weight>(entity))
			return registry.get<Weight>(entity).weight;
		return 0;
	}

	std::map<std::string, std::string> get_info(const entt::registry& registry, const entt::entity entity)
	{
		std::map<std::string, std::string> info;
		if (registry.all_of<Faction>(entity))
			info["faction"] = registry.get<Faction>(entity).faction;
		if (registry.all_of<Level>(entity))
			info["level"] = std::to_string(registry.get<Level>(entity).level);
		info["category"] = registry.get<Category>(entity).category;
		info["subcategory"] = registry.get<Subcategory>(entity).subcategory;
		if (registry.any_of<Damage, Equipment>(entity))
			info["damage"] = std::to_string(get_damage_min(registry, entity)) + " - " + std::to_string(get_damage_max(registry, entity));
		info["weight"] = std::to_string(get_weight(registry, entity));
		if (registry.any_of<ArmorPenetration, Equipment>(entity))
			info["armor penetration"] = std::to_string(get_armor_penetration(registry, entity));
		if (registry.any_of<Armor, Equipment>(entity))
			info["armor"] = std::to_string(get_armor(registry, entity));
		if (registry.any_of<Accuracy, Equipment>(entity))
			info["accuracy"] = std::to_string(get_accuracy(registry, entity));
		if (registry.any_of<Evasion, Equipment>(entity))
			info["evasion"] = std::to_string(get_evasion(registry, entity));
		if (registry.any_of<Barrier, Equipment>(entity))
			info["barrier"] = std::to_string(get_barrier(registry, entity));
		if (registry.any_of<Power, Equipment>(entity))
			info["power"] = std::to_string(get_power(registry, entity));
		if (registry.any_of<CritChance, Equipment>(entity))
			info["crit chance"] = std::to_string(get_crit_chance(registry, entity));
		if (registry.any_of<CritMultiplier, Equipment>(entity))
			info["crit multiplier"] = std::to_string(get_crit_multiplier(registry, entity));
		if (registry.all_of<Attributes>(entity))
		{
			info["strength"] = std::to_string(get_strength(registry, entity));
			info["dexterity"] = std::to_string(get_dexterity(registry, entity));
			info["intelligence"] = std::to_string(get_intelligence(registry, entity));
			info["health"] = std::to_string(registry.get<Resources>(entity).health) + " / " + std::to_string(get_health_max(registry, entity));
			info["fatigue"] = std::to_string(registry.get<Resources>(entity).fatigue) + " / " + std::to_string(get_fatigue_max(registry, entity));
			info["mana"] = std::to_string(registry.get<Resources>(entity).mana) + " / " + std::to_string(get_mana_max(registry, entity));
		}
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
		const auto& subcategory = registry.get<Subcategory>(entity).subcategory;
		return std::find(equippable.begin(), equippable.end(), subcategory) != equippable.end();
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

	size_t get_level(const entt::registry& registry, const entt::entity entity)
	{
		if (!registry.all_of<Level>(entity))
			return 0;
		const auto& level = registry.get<Level>(entity).level;
		return level;
	}

	bool are_enemies(const entt::registry& registry, const entt::entity a, const entt::entity b)
	{
		if (!registry.all_of<Faction>(a) || !registry.all_of<Faction>(b))
			return false;
		return registry.get<Faction>(a).faction != registry.get<Faction>(b).faction;
	}

	bool is_dead(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Resources>(entity))
			return registry.get<Resources>(entity).health <= 0;
		return false;
	}
};
