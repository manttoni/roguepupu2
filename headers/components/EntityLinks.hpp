#pragma once

#include <vector>
#include "external/entt/fwd.hpp"

// Create a bunch of ECS components for different purposes but same method

struct EntityLinks
{
	std::vector<entt::entity> entities;

	void add_link(const entt::entity entity);
	void remove_link(const entt::entity entity);
	bool empty() const;

	friend std::ostream& operator<<(std::ostream& os, const EntityLinks& el);
};

// Events can have consequences (or Effects), defined in these
struct OnDamage : EntityLinks{};
struct OnDeath : EntityLinks{};
struct OnDestroy : EntityLinks{};
struct OnGather : EntityLinks{};
struct OnMove : EntityLinks{};
struct OnEnter : EntityLinks{};
struct OnHit : EntityLinks{};
struct OnOpen : EntityLinks{};
struct OnUse : EntityLinks{};

// Spells
struct OnCast : EntityLinks{};
struct OnUpcast : EntityLinks{};
struct Projectiles : EntityLinks{};

// Containers
struct Inventory : EntityLinks{};
struct EquippedItems : EntityLinks{}; // Owner of links is probably a bodypart

// Bodyparts (or limbs) can hold items
struct Bodyparts : EntityLinks{};
struct EquippedIn : EntityLinks{}; // define what bodyparts can hold the owner of these links

/* Sword, EquippedIn: "hand"
 * Hand, EquippedItems: [ "sword" ]
 * */

struct Lights : EntityLinks{};
struct AIBehaviors : EntityLinks{};
struct SpawnConditions : EntityLinks{};
struct DamageRolls : EntityLinks{};
struct WorldObjects : EntityLinks{};

