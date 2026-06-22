#include "domain/Dice.hpp"

int Dice::roll(const int advantage) const
{
	if (amount == 0 || sides == 0)
		return 0;
	int result = 0;
	for (size_t i = 0; i < amount; ++i)
		result += Random::rand<size_t>(1, sides);

	for (int i = advantage; i != 0; i += advantage / std::abs(advantage))
	{
		int other_result = roll();
		if (other_result < result && advantage < 0)
			result = other_result;
		if (other_result > result && advantage > 0)
			result = other_result;
	}

	return result;
}

