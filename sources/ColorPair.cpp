#include <ncurses.h>      // for init_pair
#include "Color.hpp"      // for Color
#include "ColorPair.hpp"  // for ColorPair
#include "UI.hpp"         // for UI

ColorPair::ColorPair(const Color& fg, const Color& bg) : fg(fg), bg(bg)
{}

short ColorPair::init() const
{
	static short pair_id_counter = 1;
	short pair_id = pair_id_counter;
	pair_id_counter++;
	if (pair_id_counter >= 256)
		pair_id_counter = 1;

	short fg_id = fg.init();
	short bg_id = bg.init();
	init_pair(pair_id, fg_id, bg_id);
	UI::instance().set_initialized_color_pair(*this, pair_id);
	return pair_id;
}

bool ColorPair::operator==(const ColorPair& other) const
{
	return fg == other.fg && bg == other.bg;
}

bool ColorPair::operator<(const ColorPair& other) const
{
	if (fg != other.fg)
		return fg < other.fg;
	return bg < other.bg;
}
