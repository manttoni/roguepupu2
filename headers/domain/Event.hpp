#pragma once

#include "domain/Actor.hpp"
#include "domain/Effect.hpp"
#include "domain/Target.hpp"

/* Describes something that has happaned
 * "actor causes effect on target"
 * Can be logged as a message
 * "Rabdin crafts something"
 * "Rabdin ignites goblin"
 * "Rabdin moves"
 *
 * Actor and Target are the same thing with different type,
 * to force type
 * */

struct Event
{
	Actor actor{};		// who/what does from where
	Effect effect{};	// what they do
	Target target{};	// to who/what/where
};
