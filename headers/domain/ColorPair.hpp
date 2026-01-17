#pragma once

#include "Color.hpp"      // for Color

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

		bool operator==(const ColorPair& other) const = default;
		bool operator!=(const ColorPair& other) const = default;
		bool operator<(const ColorPair& other) const;

		short init() const;
};
