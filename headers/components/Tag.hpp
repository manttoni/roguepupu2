#pragma once

/* If entity is valid and has a tag (created with editor),
 * it also has all necessary components to fulfill the "role".
 * Can have all, but not all combinations are tested
 * */
namespace Component::Tag
{
#define TAG_COMPONENTS(X) \
	X(Ammo) \
	X(Bodypart) \
	X(Container) \
	X(Creature) \
	X(Dead) \
	X(Destroyed) \
	X(Destructible) \
	X(Door) \
	X(Equipment) \
	X(Gatherable) \
	X(Item) \
	X(Mushroom) \
	X(NPC) \
	X(Plant) \
	X(Player) \
	X(Projectile) \
	X(Trap) \
	X(WorldObject) \
	X(SpawnCondition) \
	X(AIBehavior) \
	X(DamageRoll) \
	X(Effect) \
	X(Weapon) \
	X(WeaponPropertyVersatile) \
	X(WeaponPropertyFinesse) \
	X(WeaponPropertyThrown) \
	X(WeaponPropertyRanged) \
	X(WeaponPropertyImprovised) \
	X(WeaponPropertyMelee) \
	X(WeaponPropertyUnarmed) \
	X(WeaponPropertyLight) \
	X(WeaponPropertyHeavy) \
	X(WeaponPropertyLoading) \
	X(WeaponPropertyReach) \
	X(WeaponPropertyTwoHanded) \
	X(WeaponPropertyDestructive)

#define X(name) \
	struct name {};
	TAG_COMPONENTS(X)
#undef X
}
