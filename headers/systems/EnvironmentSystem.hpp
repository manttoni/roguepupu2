#pragma once

#include "entt.hpp"

class Cave;
namespace EnvironmentSystem
{
	void simulate_environment(entt::registry& registry);
	void simulate_liquids(entt::registry& registry);
};
