#pragma once

namespace Component::Value::Type
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

#define ENUMS(X) \
	X(Ammo,  AMMO_TYPES) \
	X(Armor,  ARMOR_TYPES) \
	X(Weapon,  WEAPON_TYPES) \
	X(Damage,  DAMAGE_TYPES) \
	X(Skill,  SKILL_TYPES) \
	X(Condition,  CONDITION_TYPES)

#define ENUM_VALUE(name) name,

#define DECLARE_ENUM(enum_name, enum_list) \
	enum class enum_name \
	{ \
		enum_list(ENUM_VALUE) \
	};

	ENUMS(DECLARE_ENUM)

#define ENUM_CASE(name) \
	case Enum::name:    \
						return #name;

#define DECLARE_TO_STRING(enum_name, enum_list)                  \
		constexpr std::string_view to_string(enum_name value)       \
	{                                                            \
		using Enum = enum_name;                                  \
		\
		switch (value)                                           \
		{                                                        \
			enum_list(ENUM_CASE)                                 \
		}                                                        \
		\
		return "Unknown";                                        \
	}

		ENUMS(DECLARE_TO_STRING)

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
}
