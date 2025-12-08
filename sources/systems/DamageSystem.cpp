#include "systems/DamageSystem.hpp"

namespace DamageSystem
{
	Type parse_type(const std::string& string)
	{
		if (string == "piercing") return Type::piercing;
		if (string == "bludgeoning") return Type::bludgeoning;
		if (string == "slashing") return Type::slashing;
		Log::error("Unknown damage type: " + string);
	}
};
