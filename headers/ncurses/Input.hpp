#pragma once

#include "utils/Vec2.hpp"

namespace Ncurses::Input
{
	enum class Key
	{
		None,

		// Direction
		Up,
		UpLeft,
		UpRight,
		Down,
		DownLeft,
		DownRight,
		Left,
		Right,

		// ASCII
		Alphanumeric,
		Space,
		Enter,

		// Unprintable
		Backspace,
		Escape,
		Resize,
	};

	Vec2<int> to_direction(const Key key);
	inline bool is_directional(const Key key) { return key >= Key::Up && key <= Key::Right; }
	inline bool is_ascii(const Key key) { return key >= Key::Alphanumeric && key < Key::Enter; }
	struct Event
	{
		Key key = Key::None;
		char ch = '\0';
		bool shift = false;
	};

	Event get_event(int timeout_ms = -1);
}
