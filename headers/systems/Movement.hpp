#pragma once

#include <vector>
#include "domain/Position.hpp"
#include "external/entt/fwd.hpp"

class Cave;

                namespace System::Movement

{
	std::vector<Domain::Position> find_path(const entt::registry& registry, const Domain::Position& start, const Domain::Position& end, const bool allow_blocked_end = true);
	Domain::Position get_first_step(const entt::registry& registry, const Domain::Position& start, const Domain::Position& end);
	bool can_move(const entt::registry& registry, const Domain::Position& from, const Domain::Position& to);
	bool can_move(const entt::registry& registry, const entt::entity entity, const Domain::Position& to);
	void move(entt::registry& registry, const entt::entity entity, const Domain::Position& dst);
};
