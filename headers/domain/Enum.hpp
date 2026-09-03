#pragma once

#include "external/magic_enum/magic_enum.hpp"

#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

namespace Enum
{
	enum class AIBehaviorType
	{
		None,
		Wander,
	};

	enum class AmmoType
	{
		None,
		Arrow,
		Bolt,
		Bullet,
		Needle,
	};

	enum class ArmorType
	{
		None,
		Light,
		Medium,
		Heavy,
	};

	enum class DamageType
	{
		None,
		Piercing,
		Slashing,
		Bludgeoning,
		Fire,
		Cold,
		Lightning,
		Thunder,
		Acid,
		Poison,
		Psychic,
		Force,
		Necrotic,
		Radiant,
	};

	enum class WeaponProperty
	{
		None,

		// D&D 5e weapon properties
		Ammunition,
		Finesse,
		Heavy,
		Light,
		Loading,
		Range,
		Reach,
		Special,
		Thrown,
		TwoHanded,
		Versatile,

		// Roguepupu properties or classifications
		Destructive,
		Improvised,
		Melee,
		Ranged,
		Unarmed,

	};

	enum class Faction
	{
		None,
		Player,
		Enemy,
	};

	enum class Rarity
	{
		None,
		Common,
		Uncommon,
		Rare,
		VeryRare,
		Legendary,
	};

	enum class NaturalGrowthType
	{
		None,
		Plant,
		Mushroom,
	};

	enum class CreatureType
	{
		None,
		Humanoid,
	};

	enum class Race
	{
		None,
		Human,
		Elf,
		Dwarf,
		Goblin,
	};

	enum class WeaponType
	{
		None,

		// Simple melee weapons
		Club,
		Dagger,
		Greatclub,
		Handaxe,
		Javelin,
		LightHammer,
		Mace,
		Quarterstaff,
		Sickle,
		Spear,

		// Simple ranged weapons
		LightCrossbow,
		Dart,
		Shortbow,
		Sling,

		// Martial melee weapons
		Battleaxe,
		Flail,
		Glaive,
		Greataxe,
		Greatsword,
		Halberd,
		Lance,
		Longsword,
		Maul,
		Morningstar,
		Pike,
		Rapier,
		Scimitar,
		Shortsword,
		Trident,
		WarPick,
		Warhammer,
		Whip,

		// Martial ranged weapons
		Blowgun,
		HandCrossbow,
		HeavyCrossbow,
		Longbow,
		Net
	};

	enum class SkillType
	{
		None,

		Acrobatics,
		AnimalHandling,
		Arcana,
		Athletics,
		Deception,
		History,
		Insight,
		Intimidation,
		Investigation,
		Medicine,
		Nature,
		Perception,
		Performance,
		Persuasion,
		Religion,
		SleightOfHand,
		Stealth,
		Survival
	};

	template<typename E>
		concept GameEnum =
		std::is_enum_v<E> &&
		requires
		{
			E::None;
		};

	template<GameEnum E>
		[[nodiscard]]
		constexpr std::string_view to_string(const E value)
		{
			const auto name = magic_enum::enum_name(value);

			if (name.empty())
				return "Unknown";

			return name;
		}

	template<GameEnum E>
		[[nodiscard]]
		E from_string(const std::string_view value)
		{
			const auto result = magic_enum::enum_cast<E>(value);

			if (!result)
			{
				throw std::invalid_argument{
					"Invalid enum value: " + std::string{value}};
			}

			return *result;
		}

	template<GameEnum E>
		std::ostream& operator<<(
				std::ostream& output,
				const E value)
		{
			return output << to_string(value);
		}

} // namespace Enum
