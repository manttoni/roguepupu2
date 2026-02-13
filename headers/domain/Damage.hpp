#pragma once

#include <limits>
#include <unordered_map>
#include "domain/Color.hpp"
#include "utils/Range.hpp"
#include "utils/Error.hpp"
#include "utils/Random.hpp"

namespace Damage
{
	enum class Type
	{
		None,
		Piercing,
		Slashing,
		Bludgeoning,
		Burning,
		Bleeding,
		Poison,
	};

	inline std::string type_to_string(const Type type)
	{
		switch (type)
		{
			case Type::Piercing: return "piercing";
			case Type::Slashing: return "slashing";
			case Type::Bludgeoning: return "bludgeoning";
			case Type::Burning: return "burning";
			case Type::Bleeding: return "bleeding";
			case Type::Poison: return "poison";
			default: Error::fatal("Unhandled damage type");
		}
	}

	inline Type string_to_type(const std::string& str)
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

	inline Color get_color(const Type type)
	{
		switch (type)
		{
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

	struct Roll
	{
		Type type = Type::None;
		double result = std::numeric_limits<double>::quiet_NaN();

		Roll() = default;
		Roll(const Type type, const size_t result) : type(type), result(static_cast<double>(result)) {}

		std::string to_string() const
		{
			return std::to_string(static_cast<size_t>(result)) + " " + type_to_string(type);
		}
	};

	struct Spec
	{
		Type type = Type::None;
		Range<size_t> range;

		Spec() = default;
		Spec(const Type type, const Range<size_t>& range) : type(type), range(range) {}

		Roll roll() const
		{
			return Roll(type, Random::rand<size_t>(range));
		}

		bool operator==(const Spec& other) const = default;
		bool operator!=(const Spec& other) const = default;
		bool operator<(const Spec& other) const
		{
			return range.average() < other.range.average();
		}
		bool operator>(const Spec& other) const
		{
			return range.average() > other.range.average();
		}
	};

};
