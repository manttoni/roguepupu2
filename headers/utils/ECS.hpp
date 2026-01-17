#pragma once

#include <optional>
#include "external/entt/entt.hpp"
#include "domain/Cave.hpp"
#include "domain/Color.hpp"
#include "infrastructure/GameState.hpp"
#include "systems/position/PositionSystem.hpp"
#include "components/Components.hpp"
#include "utils/Utils.hpp"
#include "domain/Event.hpp"
#include "domain/EventQueue.hpp"

namespace ECS
{
	/*
	   template<typename... Components>
	   std::vector<entt::entity> get_entities_in_cave(const entt::registry& registry, const size_t cave_idx, const Components&... values)
	   {
	   std::vector<entt::entity> result;
	   auto view = registry.view<std::decay_t<Components>...>();

	   for (auto entity : view)
	   {
	   if (!registry.all_of<Position>(entity) || registry.get<Position>(entity).cave_idx != cave_idx)
	   continue;
	   if ((registry.get<std::decay_t<Components>>(entity) == values) && ...)
	   result.push_back(entity);
	   }

	   return result;
	   }
	   */
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

	inline entt::entity get_player(const entt::registry& registry)
	{
		return registry.ctx().get<GameState>().player;
	}

	inline Cave& get_active_cave(entt::registry& registry)
	{
		const auto player = get_player(registry);
		assert(player != entt::null);
		assert(registry.all_of<Position>(player));
		const auto& pos = registry.get<Position>(player);
		return PositionSystem::get_cave(registry, pos);
	}

	inline Color get_color(const entt::registry& registry, const entt::entity entity)
	{
		if (!registry.all_of<FGColor>(entity))
			return Color::white();
		return registry.get<FGColor>(entity).color;
	}

	inline std::string get_colored_name(const entt::registry& registry, const entt::entity entity)
	{
		const std::string& name = Utils::capitalize(registry.get<Name>(entity).name);
		const Color& fgcolor = get_color(registry, entity);
		return fgcolor.markup() + name + "{reset}";
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
};
