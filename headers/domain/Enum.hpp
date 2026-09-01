#pragma once

#include <string_view>

namespace Enum
{
#define AMMO_TYPES(X) \
	X(Arrow) X(Bolt) X(Bullet) X(Needle)

#define ARMOR_TYPES(X) \
	X(Light) X(Medium) X(Heavy)

#define WEAPON_TYPES(X) \
	X(Club) X(Dagger) X(Greatclub) X(Handaxe) X(Javelin) X(LightHammer) X(Mace) \
	X(Quarterstaff) X(Sickle) X(Spear) X(CrossbowLight) X(Dart) X(Shortbow) \
	X(Sling) X(Battleaxe) X(Flail) X(Glaive) X(Greataxe) X(Greatsword) X(Halberd) \
	X(Lance) X(Longsword) X(Maul) X(Morningstar) X(Pike) X(Rapier) X(Scimitar) \
	X(Shortsword) X(Trident) X(Warpick) X(Warhammer) X(Whip) X(Blowgun) \
	X(CrossbowHand) X(CrossbowHeavy) X(Longbow) X(Net)

#define DAMAGE_TYPES(X) \
	X(Piercing) X(Slashing) X(Bludgeoning) \
	X(Fire) X(Cold) X(Lightning) X(Thunder) X(Acid) \
	X(Poison) X(Psychic) X(Force) X(Necrotic) X(Radiant)

#define SKILL_TYPES(X) \
	X(Acrobatics) X(AnimalHandling) X(Arcana) X(Athletics) \
	X(Deception) X(History) X(Insight) X(Intimidation) \
	X(Investigation) X(Medicine) X(Nature) X(Perception) \
	X(Performance) X(Persuasion) X(Religion) X(SleightOfHand) \
	X(Stealth) X(Survival)

#define CONDITION_TYPES(X) \
	X(Blinded) X(Charmed) X(Deafened) X(Frightened) X(Grappled) \
	X(Incapacitated) X(Invisible) X(Paralyzed) X(Petrified) \
	X(Poisoned) X(Prone) X(Restrained) X(Stunned) X(Unconscious) \
	X(Exhaustion)

#define WEAPON_PROPERTIES(X) \
	X(Versatile) \
	X(Finesse) \
	X(Thrown) \
	X(Ranged) \
	X(Improvised) \
	X(Melee) \
	X(Unarmed) \
	X(Light) \
	X(Heavy) \
	X(Loading) \
	X(Reach) \
	X(TwoHanded) \
	X(Destructive)

#define AIBEHAVIOR_TYPES(X) \
	X(Attack) \
	X(Wander) \
	X(Flee)

#define DEATH_TYPES(X) \
	X(Dead)

#define CREATURE_TYPES(X) \
	X(Creature)

#define RACES(X) \
	X(Human) X(Elf) X(Dwarf)

#define RARITIES(X) \
	X(Common) X(Uncommon) X(Rare) X(VeryRare) X(Legendary)

#define NATURALGROWTH_TYPES(X) \
	X(Mushroom) X(Plant) X(Moss)

#define FACTIONS(X) \
	X(Player) X(Goblins)

#define ENUMS(X) \
	X(AmmoType,  AMMO_TYPES) \
	X(ArmorType,  ARMOR_TYPES) \
	X(WeaponType,  WEAPON_TYPES) \
	X(DamageType,  DAMAGE_TYPES) \
	X(SkillType,  SKILL_TYPES) \
	X(ConditionType,  CONDITION_TYPES) \
	X(WeaponProperty, WEAPON_PROPERTIES) \
	X(AIBehaviorType, AIBEHAVIOR_TYPES) \
	X(DeathType, DEATH_TYPES) \
	X(CreatureType, CREATURE_TYPES) \
	X(Race, RACES) \
	X(Rarity, RARITIES) \
	X(NaturalGrowthType, NATURALGROWTH_TYPES) \
	X(Faction, FACTIONS)

#define ENUM_VALUE(name) name,

#define DECLARE_ENUM(enum_name, enum_list) \
	enum class enum_name \
	{ \
		None, \
		enum_list(ENUM_VALUE) \
		Count \
	};

	ENUMS(DECLARE_ENUM)

#define ENUM_CASE(name) \
	case E::name:    \
					 return #name;

#define DECLARE_TO_STRING(enum_name, enum_list)                  \
		constexpr std::string_view to_string(enum_name value)       \
	{                                                            \
		using E = enum_name;                                  \
		switch (value)                                           \
		{                                                        \
			case E::None: \
						  return "None"; \
			case E::Count: \
						   return "Count"; \
			enum_list(ENUM_CASE)                                 \
		}                                                        \
		return "Unknown";                                        \
	}

		ENUMS(DECLARE_TO_STRING)

#define DECLARE_ENUM_STREAM(enum_name, enum_list)             \
		inline std::ostream& operator<<(std::ostream& os, enum_name value) \
		{                                                          \
			return os << to_string(value);                          \
		}

		ENUMS(DECLARE_ENUM_STREAM)

#undef DECLARE_ENUM_STREAM
#undef DECLARE_TO_STRING
#undef ENUM_CASE
#undef DECLARE_ENUM
#undef ENUM_VALUE
#undef ENUMS
#undef AMMO_TYPES
#undef ARMOR_TYPES
#undef WEAPON_TYPES
#undef DAMAGE_TYPES
#undef SKILL_TYPES
#undef CONDITION_TYPES
#undef WEAPON_PROPERTIES
#undef AIBEHAVIOR_TYPES
#undef DEATH_TYPES
#undef CREATURE_TYPES
#undef RACES
#undef RARITIES
#undef NATURALGROWTH_TYPES
#undef FACTIONS
}
