#pragma once

#include "external/entt/fwd.hpp"
#include "domain/Position.hpp"

namespace System::Vision

{
	bool has_line_of_sight(
			const entt::registry& registry,
			const Domain::Position& a,
			const Domain::Position& b);

	bool has_vision(
			const entt::registry& registry,
			const Domain::Position& a,
			const Domain::Position& b,
			const double distance = double {});
	bool has_vision(
			const entt::registry& registry,
			const entt::entity a,
			const entt::entity b);
	bool has_vision(
			const entt::registry& registry,
			const entt::entity a,
			const Domain::Position& b);
	bool has_vision(
			const entt::registry& registry,
			const Domain::Position& a,
			const entt::entity b);

	std::vector<Domain::Position> get_visible_positions(
			const entt::registry& registry,
			const entt::entity entity,
			const double distance);
	std::vector<Domain::Position> get_visible_positions(
			const entt::registry& registry,
			const entt::entity entity);
	std::vector<Domain::Position> get_visible_positions(
			const entt::registry& registry,
			const Domain::Position& pos,
			const double distance);

	std::vector<entt::entity> get_visible_entities(const entt::registry& registry, const entt::entity entity, const Domain::Position& position);
	std::vector<entt::entity> get_visible_entities(const entt::registry& registry, const entt::entity entity, const double distance);
	std::vector<entt::entity> get_visible_entities(const entt::registry& registry, const entt::entity entity);
	std::vector<entt::entity> get_visible_entities(const entt::registry& registry, const Domain::Position& position, const Domain::Position& position2);
	std::vector<entt::entity> get_visible_entities(const entt::registry& registry, const Domain::Position& position, const double distance);
	std::vector<entt::entity> get_visible_entities(const entt::registry& registry, const Domain::Position& position);
}
