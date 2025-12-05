#pragma once

#include <map>
#include "entt.hpp"
#include "Utils.hpp"

namespace MovementSystem
{
	extern std::map<int, Vec2> movement_keys;
	bool movement_key_pressed(const int key);
	double move(entt::registry& registry, entt::entity entity, const int key);
	double move(entt::registry& registry, entt::entity entity, const Vec2 direction);
	double move(entt::registry& registry, entt::entity entity, Cell& cell);
};
