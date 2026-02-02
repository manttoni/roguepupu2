#pragma once

#include "external/entt/entt.hpp"
#include "domain/Cell.hpp"
#include "domain/Cave.hpp"
#include "domain/World.hpp"
#include "domain/Position.hpp"
#include "utils/ECS.hpp"
#include "utils/Vec2.hpp"
#include "utils/Error.hpp"

enum class TestCaveType
{
	Floor,	// Every cell type Floor
	Room,	// Rock on edges, floor elsewhere
};

size_t get_test_cave_idx(entt::registry& registry, const size_t size, const TestCaveType type)
{
	if (type == TestCaveType::Room)
	{
		const auto cave_idx = ECS::get_world(registry).new_cave(size, Cell::Type::Rock);

		Cave& cave = ECS::get_cave(registry, cave_idx);
		for (const auto pos : cave.get_positions())
		{
			const Vec2 coords(pos.cell_idx, cave.get_size());
			if (coords.y == 0 || static_cast<size_t>(coords.y) == cave.get_size() - 1 ||
				coords.x == 0 || static_cast<size_t>(coords.x) == cave.get_size() - 1)
				continue;
			cave.get_cell(pos).set_type(Cell::Type::Floor);
		}

		return cave_idx;
	}
	else if (type == TestCaveType::Floor)
	{
		const auto cave_idx = ECS::get_world(registry).new_cave(size, Cell::Type::Floor);
		return cave_idx;
	}
	else
		Error::fatal("Uknown TestCaveType");
}
