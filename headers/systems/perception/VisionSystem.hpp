#pragma once

#include "external/entt/fwd.hpp"
#include "domain/Position.hpp"

struct Position;
namespace VisionSystem
{
	bool has_vision(const entt::registry& registry, const entt::entity entity, const Position& position);
	bool has_vision(const entt::registry& registry, const entt::entity a, const entt::entity b);
	bool has_line_of_sight(const entt::registry& registry, const Position& a, const Position& b);
	std::vector<Position> get_visible_positions(const entt::registry& registry, const entt::entity entity);
	std::vector<entt::entity> get_visible_entities_in_position(const entt::registry& registry, const entt::entity seer, const Position& position);
	std::vector<entt::entity> get_visible_entities(const entt::registry& registry, const entt::entity entity);
};
