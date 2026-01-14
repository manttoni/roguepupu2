#include <string>                       // for basic_string, to_string, oper...
#include "Cave.hpp"                     // for Cave
#include "Cell.hpp"                     // for Cell
#include "Color.hpp"                    // for Color
#include "Components.hpp"               // for Damage, Equipment, Resources
#include "ECS.hpp"                      // for are_enemies, can_see, distance
#include "Utils.hpp"                    // for capitalize
#include "entt.hpp"                     // for allocator, entity, registry
#include "systems/EquipmentSystem.hpp"  // for is_dual_wielding, is_equipped
#include "GameLogger.hpp"
#include "GameState.hpp"
#include "DevTools.hpp"
#include "AbilityDatabase.hpp"
#include "World.hpp"
#include "RenderData.hpp"

namespace ECS
{
	entt::registry init_registry()
	{
		entt::registry registry;
		registry.ctx().emplace<GameLogger>();		// Log game events
		registry.ctx().emplace<GameState>();		// Turn number, game_running
		registry.ctx().emplace<RenderData>();		// frame, seen and visible cells
		registry.ctx().emplace<EventQueue>();		// Resolve events after actions
		registry.ctx().emplace<Dev>();				// Cheats
		registry.ctx().emplace<AbilityDatabase>();	// Parsed abilities for creatures
		registry.ctx().emplace<World>();			// Holds all caves
		return registry;
	}

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

	std::string get_name(const entt::registry& registry, const entt::entity entity)
	{
		if (entity == entt::null)
			return "null";
		return registry.get<Name>(entity).name;
	}

	wchar_t get_glyph(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Glyph>(entity))
			return registry.get<Glyph>(entity).glyph;
		return registry.get<Name>(entity).name.front();
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
		info["size"] = std::to_string(get_size(registry, entity));
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

	entt::entity get_player(const entt::registry& registry)
	{
		const auto players = registry.view<Player>();
		if (players.empty())
			Log::error("Player not found");
		return *players.begin();
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
	const Cave& get_active_cave(const entt::registry& registry)
	{
		const auto player = get_player(registry);
		const size_t idx = registry.get<Position>(player).cave_idx;
		return registry.ctx().get<World>().get_cave(idx);
	}
	Cave& get_active_cave(entt::registry& registry)
	{
		const auto player = get_player(registry);
		const size_t idx = registry.get<Position>(player).cave_idx;
		return registry.ctx().get<World>().get_cave(idx);
	}



	void destroy_entity(entt::registry& registry, const entt::entity entity)
	{
		registry.destroy(entity);
	}

	double get_size(const entt::registry& registry, const entt::entity entity)
	{
		if (!registry.all_of<Size>(entity))
			return 0.0;
		return registry.get<Size>(entity).size;
	}

	std::vector<entt::entity> get_entities(const entt::registry& registry, const Position& position)
	{
		std::vector<entt::entity> entities;
		for (const auto entity : registry.view<Position>())
		{
			if (registry.get<Position>(entity) == position)
				entities.push_back(entity);
		}
		return entities;
	}

	std::vector<entt::entity> get_entities(const entt::registry& registry, const size_t cave_idx)
	{
		std::vector<entt::entity> entities;
		for (const auto entity : registry.view<Position>())
		{
			if (registry.get<Position>(entity).cave_idx == cave_idx)
				entities.push_back(entity);
		}
		return entities;
	}



};
