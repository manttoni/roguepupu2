#pragma once

#include "external/entt/fwd.hpp"
#include "external/entt/entity/handle.hpp"
#include "domain/Position.hpp"

/* Actor is a participant in an Event
 * Event has two actors
 * */

struct Actor
{
	entt::entity entity = entt::null;
	Position position{};

	Actor() : entity(entt::null), position() {}
	Actor(entt::entity entity, Position position) : entity(entity), position(position) {}
};
