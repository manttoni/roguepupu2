#pragma once

#include <vector>
#include <string>
#include <utility>
#include "utils/Range.hpp"

struct LootTable
{
	std::string id = "";
	std::vector<std::string> item_ids;
	std::vector<double> weights;
	Range<size_t> amount{SIZE_MAX, SIZE_MAX};
	double chance = 1.0;
};
