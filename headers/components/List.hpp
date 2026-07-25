#pragma once

#include <vector>
#include "external/entt/fwd.hpp"

// Create a bunch of ECS components for different purposes but same method

namespace Component::List
{
	template <typename T>
	struct List
	{
		std::vector<T> values;
	};

#define LIST_COMPONENTS(X) \
	X(OnDamage, entt::entity) \
	X(OnDeath, entt::entity) \
	X(OnDestroy, entt::entity) \
	X(OnGather, entt::entity) \
	X(OnMove, entt::entity) \
	X(OnEnter, entt::entity) \
	X(OnHit, entt::entity) \
	X(OnOpen, entt::entity) \
	X(OnUse, entt::entity) \
	X(OnCast, entt::entity) \
	X(OnUpcast, entt::entity) \
	X(Projectiles, entt::entity) \
	X(Inventory, entt::entity) \
	X(EquippedItems, entt::entity) \
	X(Bodyparts, entt::entity) \
	X(EquippedIn, entt::entity) \
	X(Lights, entt::entity) \
	X(AIBehaviors, entt::entity) \
	X(SpawnConditions, entt::entity) \
	X(DamageRolls, entt::entity) \
	X(WorldObjects, entt::entity) \

#define X(name, type) \
	struct name : List<type>{};
	LIST_COMPONENTS(X)
#undef X
}
