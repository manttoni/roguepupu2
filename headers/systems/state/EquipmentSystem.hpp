#pragma once

#include "external/entt/fwd.hpp"

namespace EquipmentSystem
{
	bool can_equip(const entt::registry& registry, const entt::entity entity, const entt::entity item);
	bool can_unequip(const entt::registry& registry, const entt::entity entity, const entt::entity item);
	std::vector<entt::entity> get_weapons(const entt::registry& registry, const entt::entity entity);
	void equip(entt::registry& registry, const entt::entity entity, const entt::entity equipment);
	void unequip(entt::registry& registry, const entt::entity entity, const entt::entity equipment);
}
