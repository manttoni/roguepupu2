#include <string>                       // for basic_string, to_string, oper...
#include "Cave.hpp"                     // for Cave
#include "Cell.hpp"                     // for Cell
#include "Color.hpp"                    // for Color
#include "Components.hpp"               // for Damage, Equipment, Resources
#include "ECS.hpp"                      // for are_enemies, can_see, distance
#include "Utils.hpp"                    // for capitalize
#include "entt.hpp"                     // for allocator, entity, registry
#include "systems/EquipmentSystem.hpp"  // for is_dual_wielding, is_equipped

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
		return get_strength(registry, entity) * 5;
	}
	int get_fatigue_max(const entt::registry& registry, const entt::entity entity)
	{
		return get_dexterity(registry, entity) * 5;
	}
	int get_mana_max(const entt::registry& registry, const entt::entity entity)
	{
		return get_intelligence(registry, entity) * 5;
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
		info["weight"] = std::to_string(get_weight(registry, entity));
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
	Cave* get_active_cave(const entt::registry& registry)
	{
		const auto player = get_player(registry);
		return get_cell(registry, player)->get_cave();
	}

	entt::entity get_source(const entt::registry& registry, const Cave& cave)
	{
		const size_t idx = cave.get_source_idx();
		const auto& entities = cave.get_cell(idx).get_entities();
		for (const auto e : entities)
		{
			if (registry.get<Name>(e).name == "source")
				return e;
		}
		return entt::null;
	}
	entt::entity get_sink(const entt::registry& registry, const Cave& cave)
	{
		const size_t idx = cave.get_sink_idx();
		const auto& entities = cave.get_cell(idx).get_entities();
		for (const auto e : entities)
		{
			if (registry.get<Name>(e).name == "sink")
				return e;
		}
		return entt::null;
	}

	void destroy_entity(entt::registry& registry, const entt::entity entity)
	{
		Cave* cave = get_cell(registry, entity)->get_cave();
		auto& npcs = cave->get_npcs();
		auto it = std::find(npcs.begin(), npcs.end(), entity);
		if (it != npcs.end())
			npcs.erase(it);
		registry.destroy(entity);
	}
};
