#pragma once

#include <optional>
#include <string>

struct Conditions
{
	std::optional<double> weight_min, weight_max;
	std::optional<std::string> category, category_not;
};
