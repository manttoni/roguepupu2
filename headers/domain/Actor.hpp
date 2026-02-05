#pragma once

#include "external/entt/fwd.hpp"
#include "external/entt/entity/handle.hpp"
#include "domain/Position.hpp"

/* Actor is a participant in an Event
 * Event struct:
 * Actor: the one who had a turn and did something
 * Effect: What they did
 * Target: To who
 *
 * Actor and Target structs have exactly the same values,
 * entity and position
 * */

struct Actor
{
	entt::entity entity = entt::null;
	Position position{};

	bool operator==(const Actor& other) const = default;
	bool operator!=(const Actor& other) const = default;
};
