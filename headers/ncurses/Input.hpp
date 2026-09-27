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

	struct Event
	{
		Key key = Key::None;
		char ch = '\0';
		bool shift = false;

		inline bool is_directional() const
		{
			return key >= Key::Up && key <= Key::Right;
		}

		inline bool is_alphanumeric() const
		{
			return key == Key::Alphanumeric;
		}

		inline bool is_ascii() const
		{
			return key >= Key::Alphanumeric && key <= Key::Enter;
		}

		inline bool is_unprintable() const
		{
			return key >= Key::Backspace && key <= Key::Resize;
		}

		Vec2<int> to_direction() const;
	};

	Event get_event(int timeout_ms = -1);
}
