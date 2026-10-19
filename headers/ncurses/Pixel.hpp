#pragma once

#include <ncurses.h>
#include "ncurses/Color.hpp"
#include "ncurses/Attribute.hpp"

namespace Ncurses
{
	struct Pixel
	{
		char glyph{'\0'};
		Color fgcolor{};
		Color bgcolor{};
		Attribute attribute{};
	};
}
