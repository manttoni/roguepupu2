#pragma once

#include <vector>
#include "entt.hpp"
#include "Utils.hpp"
#include "Cell.hpp"
#include "Cave.hpp"

namespace MovementSystem
{
	std::vector<size_t> find_path(Cell& start, Cell& end);
	std::vector<size_t> find_path(Cave* cave, const size_t start, const size_t end);
	Cell* get_first_step(const entt::registry& registry, const entt::entity entity, Cell& dst);
	bool can_move(Cave& cave, const size_t from_idx, const size_t to_idx);
	void move(entt::registry& registry, entt::entity entity, Cell* cell);
};
