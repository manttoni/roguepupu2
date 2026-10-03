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
		Alphabetic,
		Number,
		Symbol,
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
			return key == Key::Alphabetic || key == Key::Number;
		}

		inline bool is_ascii() const
		{
			return key >= Key::Alphabetic && key <= Key::Enter;
		}

		inline bool is_unprintable() const
		{
			return key >= Key::Backspace && key <= Key::Resize;
		}

		Vec2<int> to_direction() const;
	};
	Key translate_key(int raw_key);
	char translate_character(int raw_key);
	bool is_shift_modified(int raw_key);
	Event get_event(int timeout_ms = -1);
}
