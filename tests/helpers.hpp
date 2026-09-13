#pragma once

#include <sstream>
#include <algorithm>
#include "domain/Cave.hpp"
#include "domain/Cell.hpp"
#include "domain/Position.hpp"
#include "domain/World.hpp"
#include "external/entt/entt.hpp"
#include "utils/ECS.hpp"
#include "utils/Error.hpp"
#include "utils/Utils.hpp"
#include "utils/Vec2.hpp"


class RegistryTest : public ::testing::Test
{
	protected:
		entt::registry registry;

		void SetUp() override
		{
			ECS::init_registry(registry, true); // true = testing mode
			registry.ctx().get<GameState>().test_run = true;
		}
};

namespace TestHelpers
{
	enum class CaveType
	{
		Rock,	// Solid rock everywhere
		Floor,	// Every cell type Floor
		Room,	// Rock on edges, floor elsewhere
	};

	inline size_t get_cave_idx(entt::registry& registry, const size_t size, const CaveType type)
	{
		if (type == CaveType::Rock)
		{
			const auto cave_idx = ECS::get_world(registry).new_cave(size, Domain::Cell::Type::Rock);
			return cave_idx;
		}
		else if (type == CaveType::Room)
		{
			const auto cave_idx = ECS::get_world(registry).new_cave(size, Domain::Cell::Type::Rock);

			Domain::Cave& cave = ECS::get_cave(registry, cave_idx);
			for (const auto pos : cave.get_positions())
			{
				const Vec2 coords(pos.cell_idx, cave.get_size());
				if (coords.y == 0 || static_cast<size_t>(coords.y) == cave.get_size() - 1 ||
						coords.x == 0 || static_cast<size_t>(coords.x) == cave.get_size() - 1)
					continue;
				cave.get_cell(pos).set_type(Domain::Cell::Type::Floor);
			}

			return cave_idx;
		}
		else if (type == CaveType::Floor)
		{
			const auto cave_idx = ECS::get_world(registry).new_cave(size, Domain::Cell::Type::Floor);
			return cave_idx;
		}
		else
			Error::fatal("Uknown TestCaveType");
	}


};
