#pragma once

#include <string>
#include "external/entt/fwd.hpp"

namespace Debug
{
	std::string entity_details(const entt::registry& registry, const entt::entity entity);
};
