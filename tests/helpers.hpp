#pragma once

#include <sstream>
#include <algorithm>
#include "database/EntityFactory.hpp"
#include "domain/Cave.hpp"
#include "domain/Cell.hpp"
#include "domain/Position.hpp"
#include "domain/World.hpp"
#include "external/entt/entt.hpp"
#include "systems/rendering/RenderingSystem.hpp"
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
			ECS::init_registry(registry);
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
			const auto cave_idx = ECS::get_world(registry).new_cave(size, Cell::Type::Rock);
			return cave_idx;
		}
		else if (type == CaveType::Room)
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
		else if (type == CaveType::Floor)
		{
			const auto cave_idx = ECS::get_world(registry).new_cave(size, Cell::Type::Floor);
			return cave_idx;
		}
		else
			Error::fatal("Uknown TestCaveType");
	}

	inline std::string dump_cave(const entt::registry& registry, const size_t cave_idx, std::vector<Position> highlight = {})
	{
		std::ostringstream out;
		const auto& cave = ECS::get_cave(registry, cave_idx);
		for (const auto position : cave.get_positions())
		{
			const RenderingSystem::Visual visual = RenderingSystem::get_visual(registry, position);
			auto it = std::find(highlight.begin(), highlight.end(), position);
			if (it != highlight.end()) out << "\x1b[31m";
			if (position.cell_idx % cave.get_size() == 0)
				out << std::endl;
			out << Utils::to_utf8(visual.glyph);
			if (it != highlight.end()) out << "\x1b[0m";
		}
		return out.str();
	}
	struct FightArena
	{
		std::vector<entt::entity> entities;
		size_t cave_idx;
	};
	inline FightArena get_duel_arena(entt::registry& registry)
	{
		FightArena arena;
		arena.entities = EntityFactory::instance().create_entities(registry, "test_creature", 2);
		arena.cave_idx = TestHelpers::get_cave_idx(registry, 10, TestHelpers::CaveType::Room);
		const auto& cave = ECS::get_cave(registry, arena.cave_idx);

		// Give them positions and opposing alignments and aggressive AI
		const auto mid_pos = cave.middle_position();
		registry.emplace<Position>(arena.entities[0], mid_pos);
		const Alignment a(Alignment::Type::LawfulGood);
		registry.emplace_or_replace<Alignment>(arena.entities[0], a);
		registry.emplace_or_replace<AI>(arena.entities[0]);
		registry.get<AI>(arena.entities[0]).aggressive = true;

		const auto left = Position(mid_pos.cell_idx - 1, arena.cave_idx);
		registry.emplace<Position>(arena.entities[1], left);
		const Alignment b(Alignment::Type::ChaoticEvil);
		registry.emplace_or_replace<Alignment>(arena.entities[1], b);
		registry.emplace_or_replace<AI>(arena.entities[1]);
		registry.get<AI>(arena.entities[1]).aggressive = true;

		return arena;
	}
};
