#include <stdlib.h>   // for atoi
#include <algorithm>  // for max, min
#include <cstring>    // for strchr
#include <random>     // for uniform_int_distribution, random_device, mt19937
#include <string>     // for basic_string, stoi, string
#include "Dice.hpp"   // for Dice

Dice::Dice(const std::string& str) : n(0), sides(0), mod(0), string("")
{
	if (str.find('d') == std::string::npos)
		mod = std::stoi(str);
	else
	{
		n = std::stoi(str);
		const size_t sides_pos = str.find('d');
		sides = std::stoi(str.substr(sides_pos + 1));
		size_t mod_pos = str.find('+');
		if (mod_pos == std::string::npos)
			mod_pos = str.find('-');
		if (mod_pos != std::string::npos)
			mod = std::stoi(str.substr(mod_pos));
	}
	if (n > 0)
		string += std::to_string(n) + "d" + std::to_string(sides);
	if (mod != 0)
		string += (mod > 0 && n > 0 ? "+" : "") + std::to_string(mod);
}

int Dice::roll(int advantage) const
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

