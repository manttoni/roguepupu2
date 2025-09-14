#pragma once

#include "Color.hpp"

class ColorPair
{
	private:
		short id;
		Color fg, bg; // these could be also id, not Color. hmm...

	public:
		Color get_fg() const { return fg; }
		Color get_bg() const { return bg; }
		short get_id() const { return id; }

	public:
		ColorPair();
		ColorPair(const short id, const Color& fg, const Color& bg);

		void add_glow(const Color& glow);
};
