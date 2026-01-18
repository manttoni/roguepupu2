#pragma once

#include <vector>
#include "domain/Position.hpp"
#include "external/entt/fwd.hpp"

class Cave;
namespace MovementSystem
{
	std::vector<Position> find_path(const entt::registry& registry, const Position& start, const Position& end, const bool allow_blocked_end = true);
	Position get_first_step(const entt::registry& registry, const Position& start, const Position& end);
	bool can_move(const entt::registry& registry, const Position& from, const Position& to);
	void move(entt::registry& registry, const entt::entity entity, const Position& dst);
};
