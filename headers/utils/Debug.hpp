#pragma once

#include <string>
#include <vector>
#include "external/entt/fwd.hpp"

namespace Debug
{
	std::string entity_details(const entt::registry& registry, const entt::entity entity);
	std::vector<std::string> debug_text(const entt::registry& registry);
};
