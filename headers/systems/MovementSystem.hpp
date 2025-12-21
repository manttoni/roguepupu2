#pragma once

#include <map>
#include "entt.hpp"
#include "Utils.hpp"
#include "Cell.hpp"

namespace MovementSystem
{
	bool can_move(Cave& cave, const size_t from_idx, const size_t to_idx);
	void move(entt::registry& registry, entt::entity entity, Cell* cell);
};
