#pragme once

#include <ncurses.h>
#include "Vec2.hpp"

namespace Screen
{
	inline size_t height()	{ return static_cast<size_t>(LINES); }
	inline size_t width()	{ return static_cast<size_t>(COLS); }

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

