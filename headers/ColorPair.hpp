#pragma once

#include "Color.hpp"

class ColorPair
{
	private:
		Color fg, bg;
		short id;

	public:
		short get_id() const { return id; }

	public:
		ColorPair();
		ColorPair(const Color& fg, const Color& bg);
};
