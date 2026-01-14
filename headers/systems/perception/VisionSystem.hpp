#pragma once

#include "external/entt/fwd.hpp"

struct Position;
namespace VisionSystem
{
	bool has_vision(const entt::registry& registry, const entt::entity entity, const Position& position);
	bool has_vision(const entt::registry& registry, const entt::entity a, const entt::entity b);
	bool has_line_of_sight(const entt::registry& registry, const Position& a, const Position& b);
	std::vector<size_t> get_visible_cells(const entt::registry& registry, const entt::entity entity);
};
