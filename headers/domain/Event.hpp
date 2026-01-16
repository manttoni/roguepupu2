#pragma once

#include "Actor.hpp"
#include "Effect.hpp"

/* Describes something that has happaned
 * "actor causes effect on target"
 * Can be logged as a message
 * "Rabdin crafts something"
 * "Rabdin ignites goblin"
 * "Rabdin moves"
 *
 * Target can also be an area around target.position
 * */

struct Event
{
	Actor actor{};		// who/what does from where
	Effect effect{};	// what they do
	Actor target{};		// to who/what/where
};
