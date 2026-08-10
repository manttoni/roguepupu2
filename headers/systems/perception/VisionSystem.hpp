#pragma once

#include "external/entt/fwd.hpp"
#include "domain/Position.hpp"

struct Position;
namespace VisionSystem
{
	bool has_line_of_sight(
			const entt::registry& registry,
			const Position& a,
			const Position& b);

	bool has_vision(
			const entt::registry& registry,
			const Position& a,
			const Position& b,
			const double distance = double {});
	bool has_vision(
			const entt::registry& registry,
			const entt::entity a,
			const entt::entity b);
	bool has_vision(
			const entt::registry& registry,
			const entt::entity a,
			const Position& b);
	bool has_vision(
			const entt::registry& registry,
			const Position& a,
			const entt::entity b);

	std::vector<Position> get_visible_positions(
			const entt::registry& registry,
			const entt::entity entity,
			const double distance);
	std::vector<Position> get_visible_positions(
			const entt::registry& registry,
			const entt::entity entity);
	std::vector<Position> get_visible_positions(
			const entt::registry& registry,
			const Position& pos,
			const double distance);

	std::vector<entt::entity> get_visible_entities(const entt::registry& registry, const entt::entity entity, const Position& position);
	std::vector<entt::entity> get_visible_entities(const entt::registry& registry, const entt::entity entity, const double distance);
	std::vector<entt::entity> get_visible_entities(const entt::registry& registry, const entt::entity entity);
	std::vector<entt::entity> get_visible_entities(const entt::registry& registry, const Position& position, const Position& position2);
	std::vector<entt::entity> get_visible_entities(const entt::registry& registry, const Position& position, const double distance);
	std::vector<entt::entity> get_visible_entities(const entt::registry& registry, const Position& position);
}
