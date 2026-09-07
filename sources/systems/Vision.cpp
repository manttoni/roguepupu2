#include <assert.h>
#include <bits/std_abs.h>
#include <stddef.h>
#include <algorithm>
#include <vector>

#include "systems/Vision.hpp"
#include "external/entt/entt.hpp"
#include "utils/ECS.hpp"
#include "systems/State.hpp"
#include "components/Component.hpp"
#include "domain/Cell.hpp"
#include "domain/Cave.hpp"
#include "domain/Position.hpp"
#include "external/entt/entity/fwd.hpp"


namespace System::Vision

{
	bool has_line_of_sight(const entt::registry& registry, const Domain::Position& a, const Domain::Position& b)
	{
		if (a.cave_idx != b.cave_idx)
			return false;

		const Domain::Cave& cave = ECS::get_cave(registry, a.cave_idx);

		size_t size = cave.get_size();
		double x0 = static_cast<double>(a.cell_idx % size) + 0.5;
		double y0 = static_cast<double>(a.cell_idx / size) + 0.5;
		double x1 = static_cast<double>(b.cell_idx % size) + 0.5;
		double y1 = static_cast<double>(b.cell_idx / size) + 0.5;

		double dx = x1 - x0;
		double dy = y1 - y0;

		int steps = static_cast<int>(std::max(std::abs(dx), std::abs(dy)));

		if (steps == 0)
			return true;

		double x_inc = dx / steps;
		double y_inc = dy / steps;

		double x = x0;
		double y = y0;

		for (int i = 0; i <= steps; ++i)
		{
			int ix = static_cast<int>(x);
			int iy = static_cast<int>(y);
			size_t idx = iy * size + ix;

			// If endpoint is rock, it should be visible. Same with start, to be symmetrical
			if (idx != a.cell_idx && idx != b.cell_idx
					&& ECS::blocks_vision(registry, Domain::Position(idx, a.cave_idx)))
				return false;

			x += x_inc;
			y += y_inc;
		}

		return true;
	}

	bool has_vision(const entt::registry& registry, const Domain::Position& a, const Domain::Position& b, const double distance)
	{
		return has_line_of_sight(registry, a, b) && distance >= ECS::distance(registry, a, b);
	}
	bool has_vision(const entt::registry& registry, const entt::entity a, const Domain::Position& b)
	{
		return has_vision(registry, registry.get<Domain::Position>(a), b, registry.get<Component::Value::VisionRange>(a).value);
	}
	bool has_vision(const entt::registry& registry, const Domain::Position& a, const entt::entity b)
	{
		return has_vision(registry, a, registry.get<Domain::Position>(b));
	}
	bool has_vision(const entt::registry& registry, const entt::entity a, const entt::entity b)
	{
		// TODO: b can be invisible etc...
		return has_vision(registry, a, registry.get<Domain::Position>(b));
	}

	std::vector<Domain::Position> get_visible_positions(
			const entt::registry& registry,
			const Domain::Position& position,
			const double distance)
	{
		std::vector<Domain::Position> visible;
		const auto& cave = ECS::get_cave(registry, position.cave_idx);
		for (const auto& n : cave.get_nearby_positions(position, distance))
		{
			if (has_vision(registry, position, n, distance))
				visible.push_back(n);
		}
		visible.push_back(position); // Domain::Position of observer is visible to the observer
		return visible;
	}
	std::vector<Domain::Position> get_visible_positions(
			const entt::registry& registry,
			const entt::entity entity,
			const double distance)
	{
		return get_visible_positions(registry, registry.get<Domain::Position>(entity), distance);
	}
	std::vector<Domain::Position> get_visible_positions(
			const entt::registry& registry,
			const entt::entity entity)
	{
		return get_visible_positions(registry, entity, registry.get<Component::Value::VisionRange>(entity).value);
	}
	std::vector<entt::entity> get_visible_entities(const entt::registry& registry, const Domain::Position& position)
	{
		return ECS::get_entities(registry, position);
	}
	std::vector<entt::entity> get_visible_entities(const entt::registry& registry, const Domain::Position& position, const Domain::Position& position2)
	{
		if (has_vision(registry, position, position2))
			return get_visible_entities(registry, position2);
		return {};
	}
	std::vector<entt::entity> get_visible_entities(const entt::registry& registry, const entt::entity entity, const Domain::Position& position)
	{
		if (!registry.all_of<Domain::Position>(entity))
			return {};
		return get_visible_entities(registry, registry.get<Domain::Position>(entity), position);
	}
	std::vector<entt::entity> get_visible_entities(const entt::registry& registry, const Domain::Position& position, const double distance)
	{
		std::vector<entt::entity> entities;
		for (const auto& p : get_visible_positions(registry, position, distance))
		{
			const auto ep = get_visible_entities(registry, p);
			entities.insert(entities.end(), ep.begin(), ep.end());
		}
		return entities;
	}
	std::vector<entt::entity> get_visible_entities(const entt::registry& registry, const entt::entity entity, const double distance)
	{
		return get_visible_entities(registry, registry.get<Domain::Position>(entity), distance);
	}
	std::vector<entt::entity> get_visible_entities(const entt::registry& registry, const entt::entity entity)
	{
		return get_visible_entities(registry, entity, registry.get<Component::Value::VisionRange>(entity).value);
	}



}
