#pragma once

#include "Cave.hpp"
#include "Cell.hpp"
#include "entt.hpp"
struct Position;
namespace PositionSystem
{
	double distance(const Cave& cave, const Cell& a, const Cell& b);
	double distance(const Cave& cave, const size_t a, const size_t b);
	double distance(const entt::registry& registry, const Position& a, const Position& b);
	double distance(const entt::registry& registry, const entt::entity a, const entt::entity b);

	Cell& get_cell(const entt::registry& registry, const Position& position);
	Cell& get_cell(const entt::registry& registry, const entt::entity entity);
	Cave& get_cave(const entt::registry& registry, const Position& position);
	Cave& get_cave(const entt::registry& registry, const size_t cave_idx);
};
