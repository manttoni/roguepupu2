#include "domain/Damage.hpp"

namespace Damage
{
	std::string type_to_string(const Type type)
	{
		std::ostringstream oss;
		oss << type;
		return oss.str();
	}

	Type string_to_type(const std::string& str)
	{
		static const std::unordered_map<std::string, Type> lookup = {
			{"piercing", Type::Piercing},
			{"slashing", Type::Slashing},
			{"bludgeoning", Type::Bludgeoning},
			{"burning", Type::Burning},
			{"bleeding", Type::Bleeding},
			{"poison", Type::Poison}
		};

		auto it = lookup.find(str);
		if (it != lookup.end()) return it->second;
		Error::fatal("Unhandled damage type string: " + str);
	}

	Color get_color(const Type type)
	{
		switch (type)
		{
			case Type::None:
			case Type::Piercing:
			case Type::Slashing:
			case Type::Bludgeoning:
				return Color(500, 500, 500);
			case Type::Burning:
				return Color(400, 100, 200);
			case Type::Bleeding:
				return Color(400, 0, 0);
			case Type::Poison:
				return Color(100, 400, 100);
			default: Error::fatal("Unhandled damage type");
		}
	}

	void Damage::Roll::roll(const int advantage)
	{
		result = dice.roll(advantage) + modifier;
		result = std::max(0, result);
		is_rolled = true;
	}
};
