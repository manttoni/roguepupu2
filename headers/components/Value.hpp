#pragma once

#include "Type.hpp"

namespace Component::Value
{
	template <typename T>
		struct Value
		{
			T value{};
		};

#define VALUE_COMPONENTS(X) \
		X(Strength, int) \
		X(Dexterity, int) \
		X(Constitution, int) \
		X(Intelligence, int) \
		X(Wisdom, int) \
		X(Charisma, int) \
		X(ArmorClass, int) \
		X(Initiative, int) \
		X(ProficiencyBonus, int) \
		X(HitPoints, int) \
		X(HitPointsMax, int) \
		X(MaxDexMod, int) \
		X(AttackRange, double) \
		X(Difficulty, int) \
		X(Mass, double) \
		X(Radius, double) \
		X(GoldValue, int) \
		X(Name, std::string) \
		X(Glyph, wchar_t) \
		X(CollisionMovement, bool) \
		X(CollisionVision, bool) \
		X(AmmoType, Type::Ammo) \
		X(ArmorType, Type::Armor) \
		X(WeaponType, Type::Weapon) \
		X(DamageType, Type::Damage)

#define X(name, type) \
		struct name : Value<type>{};
		VALUE_COMPONENTS(X)
#undef X
}

template <auto T> struct Proficiency{};
template <auto T> struct Expertise{};
template <Type::Damage T> struct Resistance{};
template <Type::Damage T> struct Vulnerability{};
template <auto T> struct Immunity{};
template <Type::Condition T> struct AffectedBy{};
