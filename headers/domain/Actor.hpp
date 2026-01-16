#pragma once

#include "external/entt/fwd.hpp"
#include "domain/Position.hpp"

/* Actor is a participant in an Event
 * Event has two actors
 * */

struct Actor
{
	entt::entity entity = entt::null;
	Position position{};
};
