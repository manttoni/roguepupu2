#pragma once

#include "ui/Selection.hpp"
#include "ui/Menu.hpp"

namespace UI::Search
{
	/* Wraps a Menu and enables search
	 * */
	Selection select(const Menu& menu);
}
