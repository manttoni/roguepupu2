#pragma once

#include "external/entt/entity/handle.hpp"
#include "domain/Position.hpp"

/* Target is a participant in an Event
 * Event struct:
 * Actor: the one who had a turn and did something
 * Effect: What they did
 * Target: To who
 *
 * Actor and Target structs have exactly the same values,
 * entity and position
 * */

struct Target
{
	entt::entity entity = entt::null;
	Position position{};
};
