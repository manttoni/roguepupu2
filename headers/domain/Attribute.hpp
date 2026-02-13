#pragma once

/*
enum class Attribute
{
	Strength,
	Dexterity,
	Agility,
	Perception,
	Vitality,
	Endurance,
	Willpower,
	Charisma,
	Count
};

namespace AttributeHelpers
{
	inline Attribute from_string(const std::string& string)
	{
		if (string == "strength") return Attribute::Strength;
		if (string == "dexterity") return Attribute::Dexterity;
		if (string == "agility") return Attribute::Agility;
		if (string == "perception") return Attribute::Perception;
		if (string == "vitality") return Attribute::Vitality;
		if (string == "endurance") return Attribute::Endurance;
		if (string == "willpower") return Attribute::Willpower;
		if (string == "charisma") return Attribute::Charisma;
		Error::fatal("Unknown attribute string: " + string);
	}
};
*/

struct Attributes
{
	int strength;
	int dexterity;
	int agility;
	int perception;
	int vitality;
	int endurance;
	int willpower;
	int charisma;
};

using AttributeTypes = std::tuple<
	Strength,
	Dexterity,
	Agility,
	Perception,
	Vitality,
	Endurance,
	Willpower,
	Charisma
>;
