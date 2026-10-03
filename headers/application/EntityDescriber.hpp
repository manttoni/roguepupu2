#pragma once

#include "external/entt/fwd.hpp"

class EntityDescriber
{
	public:
		EntityDescriber(const entt::registry& registry) : registry(registry) {}

		std::string describe(const entt::entity entity) const;

	private:
		const entt::registry& registry;
};
