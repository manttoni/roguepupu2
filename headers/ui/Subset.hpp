#pragma once

#include <vector>
#include <string>
#include "ui/Selection.hpp"

namespace UI::Subset
{
	Selection edit(const std::vector<std::string>& all, std::vector<std::string>& subset);
}
