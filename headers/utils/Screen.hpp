#pragma once

#include <ncurses.h>
#include "utils/Vec2.hpp"

namespace Screen
{
	inline int height()	{ return LINES; }
	inline int width()	{ return COLS; }

	inline Vec2 bot()			{ return { height(), width() / 2 }; }
	inline Vec2 botleft()		{ return { height(), 0 }; }
	inline Vec2 botright()		{ return { height(), width() }; }
	inline Vec2 left()			{ return { height() / 2, 0 }; }
	inline Vec2 middle()		{ return { height() / 2, width() / 2 }; }
	inline Vec2 right()			{ return { height() / 2, width() }; }
	inline Vec2 top()			{ return { 0, width() / 2 }; }
	inline Vec2 topleft()		{ return { 0, 0 }; }
	inline Vec2 topright()		{ return { 0, width() }; }
}

