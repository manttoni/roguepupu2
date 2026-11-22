#pragma once

#include "external/entt/fwd.hpp"

namespace Game::System::Equipment
{
	bool is_equipped(
		const entt::registry& registry,
		entt::entity entity,
		entt::entity item);

	bool equip(
		entt::registry& registry,
		entt::entity entity,
		entt::entity item);

	bool unequip(
		entt::registry& registry,
		entt::entity entity,
		entt::entity item);
}
