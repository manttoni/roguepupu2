#pragma once

#include <string>

class Dice
{
	private:
		int n, sides, mod;
	public:
		int roll(int advantage = 0);
		Dice() = default;
		Dice(const std::string& str);
		Dice(const int n, const int sides, const int mod = 0);
		Dice(const Dice& other) = default;
		Dice& operator=(const Dice& other) = default;
};
