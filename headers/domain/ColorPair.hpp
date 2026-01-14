#pragma once

#include <curses.h>       // for init_pair
#include "Color.hpp"      // for Color
#include "ColorPair.hpp"  // for ColorPair
#include "UI.hpp"         // for UI

class ColorPair
{
	private:
		Color fg, bg;

	public:
		Color get_fg() const { return fg; }
		Color get_bg() const { return bg; }

	public:
		ColorPair() = default;
		ColorPair(const Color& fg, const Color& bg);

		bool operator==(const ColorPair& other) const;
		bool operator<(const ColorPair& other) const;
		short init() const;
};
