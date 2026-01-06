#pragma once

#include <vector>
#include "entt.hpp"
#include "Cell.hpp"


namespace VisionSystem
{
	bool has_vision(const entt::registry& registry, const entt::entity entity, const Cell& cell);
	std::vector<size_t> get_visible_cells(const entt::registry& registry, const entt::entity entity);
};
