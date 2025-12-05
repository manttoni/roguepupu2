#pragma once

#include <map>
#include "entt.hpp"
#include "Utils.hpp"

namespace MovementSystem
{
	extern std::map<int, Vec2> movement_keys;
	double move(entt::registry& reg, entt::entity e, const Vec2 d);
};
