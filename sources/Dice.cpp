#include <string>
#include <cstring>
#include <random>
#include "Dice.hpp"

Dice::Dice(const std::string& str)
{
	n = std::stoi(str);
	sides = atoi(strchr(str.c_str(), 'd') + 1);
	if (strchr(str.c_str(), '-') != nullptr)
		mod = -1 * atoi(strchr(str.c_str(), '-') + 1);
	else if (strchr(str.c_str(), '+') != nullptr)
		mod = atoi(strchr(str.c_str(), '+') + 1);
}

Dice::Dice(const int n, const int sides, const int mod) : n(n), sides(sides), mod(mod) {}

int Dice::roll(int advantage)
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_int_distribution<> dist(1, sides);

	int result = dist(gen) + mod;
	while (advantage < 0)
	{
		result = std::min(result, dist(gen) + mod);
		advantage++;
	}
	while (advantage > 0)
	{
		result = std::max(result, dist(gen) + mod);
		advantage--;
	}

	return result;
}
