#pragma once

#include "systems/rendering/RenderingSystem.hpp"
#include "systems/rendering/LightingSystem.hpp"
#include <optional>
#include "components/Component.hpp"
#include "domain/Cave.hpp"
#include "domain/Color.hpp"
#include "domain/Event.hpp"
#include "domain/World.hpp"
#include "external/entt/entt.hpp"
#include "generation/CaveGenerator.hpp"
#include "infrastructure/DevSettings.hpp"
#include "infrastructure/EventQueue.hpp"
#include "infrastructure/GameLogger.hpp"
#include "infrastructure/GameSettings.hpp"
#include "infrastructure/GameState.hpp"
#include "systems/perception/VisionSystem.hpp"
#include "systems/position/TransitionSystem.hpp"
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

	inline Position get_position(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Position>(entity))
			return registry.get<Position>(entity);

		// Figure out a way to locate entity if it is carried.
		//if (registry.all_of<CarriedBy>(entity))
		//	return get_position(registry, registry.get<CarriedBy>(entity).value);

		return Position::invalid_position();
	}

	inline entt::entity get_player(const entt::registry& registry)
	{
		return registry.ctx().get<GameState>().player;
	}

	inline ::Color get_fgcolor(const entt::registry& registry, const entt::entity entity)
	{
		if (!registry.all_of<Color>(entity))
			return ::Color::white();
		return registry.get<Color>(entity);
	}

	inline std::string get_name(const entt::registry& registry, const entt::entity entity)
	{
		return registry.get<Component::Value::Name>(entity).value;
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

	inline void queue_event(entt::registry& registry, Event event)
	{
		registry.ctx().get<EventQueue>().queue.push_back(std::move(event));
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

	inline size_t get_turn_number(const entt::registry& registry)
	{
		return registry.ctx().get<GameState>().turn_number;
	}

	inline void destroy_entity(entt::registry& registry, const entt::entity entity)
	{
		registry.emplace<Component::Tag::Destroyed>(entity);

		queue_event(
				registry,
				DestroyEvent{
				.entity = entity
				});
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

	inline double distance(const entt::registry& registry, const entt::entity a, const Position& b)
	{
		return distance(registry, registry.get<Position>(a), b);
	}

	inline double distance(const entt::registry& registry, const Position& a, const entt::entity b)
	{
		return distance(registry, a, registry.get<Position>(b));
	}

	inline wchar_t get_glyph(const entt::registry& registry, const entt::entity entity)
	{
		if (registry.all_of<Component::Value::Glyph>(entity))
			return registry.get<Component::Value::Glyph>(entity).value;
		return registry.get<Component::Value::Name>(entity).value[0];
	}

	inline void init_registry(entt::registry& registry)
	{
		registry.ctx().emplace<GameState>();
		registry.ctx().emplace<GameLogger>();
		registry.ctx().emplace<World>();
		registry.ctx().emplace<RenderingSystem::Data>(); // render frame, all visuals of frame
		registry.ctx().emplace<LightingSystem::Data>(); // lightmap to be added to RenderingSystem::Visual before rendering
		registry.ctx().emplace<EventQueue>();
		registry.ctx().emplace<DevSettings>();
		registry.ctx().emplace<GameSettings>();
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

	inline bool game_running(const entt::registry& registry)
	{
		return registry.ctx().get<GameState>().game_running;
	}

	inline GameLogger::Stream game_log(entt::registry& registry)
	{
		return registry.ctx()
			.get<GameLogger>()
			.stream(registry);
	}

	inline bool weapon_has_property(const entt::registry& registry, const entt::entity weapon, const Enum::WeaponProperty property)
	{
		for (const auto p : registry.get<Component::List::WeaponProperties>(weapon))
			if (p == property)
				return true;
		return false;
	}

	inline RenderingSystem::Data get_render_data(const entt::registry& registry)
	{
		return registry.ctx().get<RenderingSystem::Data>();
	}

	inline size_t get_render_frame(const entt::registry& registry)
	{
		return get_render_data(registry).render_frame;
	}
};
