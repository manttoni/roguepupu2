#pragma once

#include <optional>
#include "components/Components.hpp"
#include "database/AbilityDatabase.hpp"
#include "database/LootTableDatabase.hpp"
#include "domain/Attribute.hpp"
#include "domain/Cave.hpp"
#include "domain/Color.hpp"
#include "domain/Event.hpp"
#include "domain/World.hpp"
#include "external/entt/entt.hpp"
#include "generation/CaveGenerator.hpp"
#include "infrastructure/DevSettings.hpp"
#include "infrastructure/EventQueue.hpp"
#include "infrastructure/EventLogger.hpp"
#include "infrastructure/GameSettings.hpp"
#include "infrastructure/GameState.hpp"
#include "systems/perception/VisionSystem.hpp"
#include "systems/position/TransitionSystem.hpp"
#include "systems/rendering/RenderData.hpp"
#include "systems/state/StateSystem.hpp"
#include "utils/Parser.hpp"
#include "utils/Random.hpp"
#include "utils/Utils.hpp"

namespace ECS
{
	/* These two functions return all entities either in a whole cave, or just in one cell in a cave
	 * */
	template <typename T = Position> std::vector<entt::entity> get_entities(const entt::registry& registry, const size_t cave_idx)
	{
		std::vector<entt::entity> entities;
		for (const auto e : registry.view<T>())
			if (registry.all_of<Position>(e) && registry.get<Position>(e).cave_idx == cave_idx)
				entities.push_back(e);
		return entities;
	}
	template <typename T = Position> std::vector<entt::entity> get_entities(const entt::registry& registry, const Position& pos)
	{
		std::vector<entt::entity> entities;
		for (const auto e : registry.view<T>())
			if (registry.all_of<Position>(e) && registry.get<Position>(e) == pos)
				entities.push_back(e);
		return entities;
	}
	template<typename T>
		T get_setting_value(const entt::registry& registry, GameSettings::Type type)
		{
			return std::get<T>(
					registry.ctx()
					.get<GameSettings>()
					.settings.at(type)
					.value
					);
		}
	inline std::vector<entt::entity> get_creatures(const entt::registry& registry, const size_t cave_idx)
	{
		std::vector<entt::entity> creatures;
		for (const auto creature : registry.view<Creature, Position>())
		{
			if (registry.get<Position>(creature).cave_idx == cave_idx)
				creatures.push_back(creature);
		}
		return creatures;
	}

	inline entt::entity get_player(const entt::registry& registry)
	{
		return registry.ctx().get<GameState>().player;
	}



	inline Color get_fgcolor(const entt::registry& registry, const entt::entity entity)
	{
		if (!registry.all_of<Color>(entity))
			return Color::white();
		return registry.get<Color>(entity);
	}

	inline std::string get_name(const entt::registry& registry, const entt::entity entity)
	{
		const std::string& name = Utils::capitalize(registry.get<Name>(entity).name);
		return name;
	}

	inline NcursesAttr get_ncurses_attr(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<NcursesAttr>(entity))
			return registry.get<NcursesAttr>(entity);
		return NcursesAttr(A_NORMAL);
	}

	inline std::string get_colored_name(const entt::registry& registry, const entt::entity entity)
	{
		const Color& fgcolor = get_fgcolor(registry, entity);
		const NcursesAttr& attr = get_ncurses_attr(registry, entity);
		std::string name = fgcolor.markup() + get_name(registry, entity) + "{reset}";
		if (attr != A_NORMAL)
		{
			name.insert(0, attr.markup());
			name += "[reset]";
		}
		return name;
	}

	inline std::vector<std::string> get_colored_names(const entt::registry& registry, const std::vector<entt::entity> entities)
	{
		std::vector<std::string> colored_names;
		for (const auto entity : entities)
			colored_names.push_back(get_colored_name(registry, entity));
		return colored_names;
	}

	inline void queue_event(entt::registry& registry, const Event& event)
	{
		registry.ctx().get<EventQueue>().queue.push_back(event);
	}

	inline World& get_world(entt::registry& registry)
	{
		return registry.ctx().get<World>();
	}

	inline Cave& get_cave(entt::registry& registry, const Position& position)
	{
		return get_world(registry).get_cave(position.cave_idx);
	}

	inline Cell& get_cell(entt::registry& registry, const Position& position)
	{
		return get_cave(registry, position).get_cell(position);
	}

	inline const World& get_world(const entt::registry& registry)
	{
		return registry.ctx().get<World>();
	}

	inline const Cave& get_cave(const entt::registry& registry, const Position& position)
	{
		return get_world(registry).get_cave(position.cave_idx);
	}

	inline const Cell& get_cell(const entt::registry& registry, const Position& position)
	{
		return get_cave(registry, position).get_cell(position);
	}

	inline const Cave& get_cave(const entt::registry& registry, const size_t cave_idx)
	{
		return registry.ctx().get<World>().get_cave(cave_idx);
	}

	inline Cave& get_cave(entt::registry& registry, const size_t cave_idx)
	{
		return registry.ctx().get<World>().get_cave(cave_idx);
	}

	inline Cave& get_active_cave(entt::registry& registry)
	{
		const auto player = get_player(registry);
		assert(player != entt::null);
		assert(registry.all_of<Position>(player));
		const auto& pos = registry.get<Position>(player);
		return get_cave(registry, pos);
	}

	inline const Cave& get_active_cave(const entt::registry& registry)
	{
		const auto player = get_player(registry);
		assert(player != entt::null);
		assert(registry.all_of<Position>(player));
		const auto& pos = registry.get<Position>(player);
		return get_cave(registry, pos);
	}

	inline entt::entity get_unlinked_passage(const entt::registry& registry, const size_t cave_idx)
	{
		(void) registry; (void) cave_idx;
		return entt::null; // function might get removed
	}

	inline size_t get_turn_number(const entt::registry& registry)
	{
		return registry.ctx().get<GameState>().turn_number;
	}

	inline void destroy_entity(entt::registry& registry, const entt::entity entity)
	{
		registry.emplace<Destroyed>(entity);

		Event destroy_event(Event::Type::Destroy);
		destroy_event.target.entity = entity;
		queue_event(registry, destroy_event);
	}

	inline double distance(const entt::registry& registry, const Position& a, const Position& b)
	{
		assert(a.cave_idx == b.cave_idx);
		return get_cave(registry, a).distance(a, b);
	}

	inline double distance(const entt::registry& registry, const entt::entity a, const entt::entity b)
	{
		assert(registry.all_of<Position>(a) && registry.all_of<Position>(b));
		return distance(registry, registry.get<Position>(a), registry.get<Position>(b));
	}

	inline wchar_t get_glyph(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Glyph>(entity))
			return registry.get<Glyph>(entity).glyph;
		return registry.get<Name>(entity).name[0];
	}

	inline double get_liquid_level(const entt::registry& registry, const Position& pos)
	{
		const auto& cell = get_cell(registry, pos);
		assert(!cell.get_liquid_mixture().empty());
		const auto depth = cell.get_effective_density();
		const auto liquid_volume = cell.get_liquid_mixture().get_volume();
		double entity_mass = 0.0;
		for (const auto e : get_entities<Mass>(registry, pos))
			entity_mass += registry.get<Mass>(e).value;

		return depth + liquid_volume + entity_mass;
	}

	inline void init_registry(entt::registry& registry)
	{
		registry.ctx().emplace<GameState>();
		registry.ctx().emplace<EventLogger>();
		registry.ctx().emplace<AbilityDatabase>();
		registry.ctx().emplace<World>();
		registry.ctx().emplace<RenderData>();
		registry.ctx().emplace<EventQueue>();
		registry.ctx().emplace<DevSettings>();
		registry.ctx().emplace<LootTableDatabase>();
		registry.ctx().emplace<GameSettings>();
	}

	inline Attributes get_attributes(const entt::registry& registry, const entt::entity entity)
	{
		using namespace StateSystem;
		Attributes attributes = {
			.strength = get_stat<Strength>(registry, entity),
			.dexterity = get_stat<Dexterity>(registry, entity),
			.constitution = get_stat<Constitution>(registry, entity),
			.intelligence = get_stat<Intelligence>(registry, entity),
			.wisdom = get_stat<Wisdom>(registry, entity),
			.charisma = get_stat<Charisma>(registry, entity)
		};
		return attributes;
	}

	inline void spawn_liquid(entt::registry& registry, const Position& position, const LiquidMixture& lm)
	{
		assert(position.is_valid());
		auto& cell = get_cell(registry, position);
		cell.get_liquid_mixture() += lm;
	}

	inline double get_light_amount(const entt::registry& registry, const Position& position)
	{
		assert(position.is_valid());
		const auto& cell = get_cell(registry, position);
		double amount = 0;
		for (const auto& [color, stacks] : cell.get_lights())
			amount += static_cast<double>(stacks) * static_cast<double>(color.get_channels_sum());
		return amount;
	}

	inline double get_liquid_amount(const entt::registry& registry, const Position& position, const Liquid::Type type)
	{
		assert(position.is_valid());
		const auto& cell = get_cell(registry, position);
		return cell.get_liquid_mixture().get_volume(type);
	}

	inline bool is_solid(const entt::registry& registry, const Position& position)
	{
		const auto& cell = get_cell(registry, position);
		if (cell.get_type() == Cell::Type::Rock)
			return true;
		for (const auto& entity : get_entities(registry, position))
		{
			if (registry.all_of<Solid>(entity))
				return true;
		}
		return false;
	}

	inline bool player_can_see_entity(const entt::registry& registry, const entt::entity e)
	{
		if (get_player(registry) == entt::null || e == entt::null)
			return false;
		return VisionSystem::has_vision(registry, ECS::get_player(registry), e);
	}

	inline bool player_can_see_position(const entt::registry& registry, const Position& position)
	{
		const auto player = get_player(registry);
		const auto can_see = VisionSystem::has_vision(registry, player, position);
		return can_see;
	}
};
