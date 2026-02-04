#pragma once

#include <optional>
#include "systems/rendering/RenderData.hpp"
#include "utils/Parser.hpp"
#include "systems/position/TransitionSystem.hpp"
#include "components/Components.hpp"
#include "domain/Cave.hpp"
#include "domain/Color.hpp"
#include "domain/Event.hpp"
#include "domain/EventQueue.hpp"
#include "domain/World.hpp"
#include "external/entt/entt.hpp"
#include "infrastructure/GameState.hpp"
#include "utils/Random.hpp"
#include "utils/Utils.hpp"
#include "generation/CaveGenerator.hpp"
#include "infrastructure/GameLogger.hpp"
#include "database/AbilityDatabase.hpp"
#include "database/LootTableDatabase.hpp"
#include "infrastructure/DevTools.hpp"

namespace ECS
{
	template<typename... Components>
		std::vector<entt::entity> get_entities(const entt::registry& registry, const Components&... values)
		{
			std::vector<entt::entity> result;
			auto view = registry.view<std::decay_t<Components>...>();

			for (const auto entity : view)
			{
				bool match = ((registry.get<std::decay_t<Components>>(entity) == values) && ...);
				if (match)
					result.push_back(entity);
			}

			return result;
		}
	template<typename... Components>
		std::vector<entt::entity> get_entities_in_cave(const entt::registry& registry, const size_t cave_idx, const Components&... values)
		{
			std::vector<entt::entity> result;
			auto view = registry.view<std::decay_t<Components>...>();

			for (const auto entity : view)
			{
				if (!registry.all_of<Position>(entity) || registry.get<Position>(entity).cave_idx != cave_idx)
					continue;
				bool match = ((registry.get<std::decay_t<Components>>(entity) == values) && ...);
				if (match)
					result.push_back(entity);
			}

			return result;
		}

	inline entt::entity get_player(const entt::registry& registry)
	{
		return registry.ctx().get<GameState>().player;
	}



	inline Color get_fgcolor(const entt::registry& registry, const entt::entity entity)
	{
		if (!registry.all_of<FGColor>(entity))
			return Color::white();
		return registry.get<FGColor>(entity).color;
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
		const auto unlinked_passages = get_entities_in_cave(registry, cave_idx, Transition{.destination = entt::null});
		assert(!unlinked_passages.empty());
		return unlinked_passages[Random::randsize_t(0, unlinked_passages.size() - 1)];
	}

	inline size_t get_turn_number(const entt::registry& registry)
	{
		return registry.ctx().get<GameState>().turn_number;
	}

	inline void destroy_entity(entt::registry& registry, const entt::entity entity)
	{
		registry.emplace<Destroyed>(entity);

		queue_event(registry, Event(
					{},
					{.type = Effect::Type::DestroyEntity},
					{.entity = entity}
					));
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
		double liquid_level = get_cell(registry, pos).get_liquid_level();
		for (const auto entity : get_entities(registry, pos))
		{
			if (registry.all_of<Size>(entity))
				liquid_level += registry.get<Size>(entity).liters;
		}
		return liquid_level;
	}

	inline size_t generate_cave(entt::registry& registry, const std::string& conf_id = "default")
	{
		const size_t cave_idx = get_world(registry).new_cave();
		CaveGenerator::Data data(registry, get_cave(registry, cave_idx));
		Parser::parse_cave_generation_conf(conf_id, data);
		CaveGenerator::generate_cave(data);
		return cave_idx;
	}

	inline void init_registry(entt::registry& registry)
	{
		registry.ctx().emplace<GameState>();
		registry.ctx().emplace<GameLogger>();
		registry.ctx().emplace<AbilityDatabase>();
		registry.ctx().emplace<World>();
		registry.ctx().emplace<RenderData>();
		registry.ctx().emplace<EventQueue>();
		registry.ctx().emplace<Dev>();
		registry.ctx().emplace<LootTableDatabase>();

	}
};
