#pragma once

#include <string>
#include "Effect.hpp"
#include "Actor.hpp"

/* Ability is a wrapper for an effect
 * Can be called a "controlled effect"
 * Can target anything within range
 * */

struct Ability
{
	std::string id = "";
	std::string category = "";
	size_t cooldown = 0;
	size_t last_used = 0;
	double range = 0.0;
	Effect effect;
};
