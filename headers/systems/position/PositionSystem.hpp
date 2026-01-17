#pragma once

#include "external/entt/fwd.hpp"

class Cell;
class Cave;
struct Position;
namespace PositionSystem
{
	// Used when registry not available, like in CaveGenerator
	double distance(const Cave& cave, const Cell& a, const Cell& b);
	double distance(const Cave& cave, const size_t a, const size_t b);

	double distance(const entt::registry& registry, const Position& a, const Position& b);
	double distance(const entt::registry& registry, const entt::entity a, const entt::entity b);

	Cell& get_cell(entt::registry& registry, const Position& position);

	Cave& get_cave(entt::registry& registry, const Position& position);
	Cave& get_cave(entt::registry& registry, const size_t cave_idx);

	const Cell& get_cell(const entt::registry& registry, const Position& position);

	const Cave& get_cave(const entt::registry& registry, const Position& position);
	const Cave& get_cave(const entt::registry& registry, const size_t cave_idx);
};
