#pragma once

#include <string>
#include "Utils.hpp"

namespace DamageSystem
{
	enum class Type
	{
		piercing,
		bludgeoning,
		slashing,
	};
	Type parse_type(const std::string& string);
};
