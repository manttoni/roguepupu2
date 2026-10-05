#pragma once

#include <vector>
#include <string>
#include "ui/Element.hpp"

namespace UI::Format
{
	std::vector<std::string> elements(const std::vector<Element::Any>& elts);
}
