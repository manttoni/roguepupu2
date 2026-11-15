#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "ui/Element.hpp"

namespace UI::Format
{
	std::size_t visible_width(const std::string& text);
	std::vector<std::string> elements(const std::vector<Element::Any>& elts);
}
