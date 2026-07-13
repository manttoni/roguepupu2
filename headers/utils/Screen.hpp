#pragma once

#include <ncurses.h>
#include "utils/Vec2.hpp"

namespace Screen
{
	inline int height()	{ return LINES; }
	inline int width()	{ return COLS; }

	inline Vec2<int> bot()			{ return { static_cast<int>(height()), static_cast<int>(width()) / 2 }; }
	inline Vec2<int> botleft()		{ return { static_cast<int>(height()), 0 }; }
	inline Vec2<int> botright()		{ return { static_cast<int>(height()), static_cast<int>(width()) }; }
	inline Vec2<int> left()			{ return { static_cast<int>(height()) / 2, 0 }; }
	inline Vec2<int> middle()		{ return { static_cast<int>(height()) / 2, static_cast<int>(width()) / 2 }; }
	inline Vec2<int> right()			{ return { static_cast<int>(height()) / 2, static_cast<int>(width()) }; }
	inline Vec2<int> top()			{ return { 0, static_cast<int>(width()) / 2 }; }
	inline Vec2<int> topleft()		{ return { 0, 0 }; }
	inline Vec2<int> topright()		{ return { 0, static_cast<int>(width()) }; }
}

