#include "ncurses/Input.hpp"

#include "utils/Log.hpp"
#include "utils/Vec2.hpp"
#include "utils/Error.hpp"
#include <climits>
#include <ncurses.h>

namespace Ncurses::Input
{
	namespace
	{
		Key translate_key(const int raw_key)
		{
			switch (raw_key)
			{
				case ERR:
					return Key::None;

				case KEY_UP:
					return Key::Up;

				case KEY_DOWN:
					return Key::Down;

				case KEY_LEFT:
					return Key::Left;

				case KEY_RIGHT:
					return Key::Right;

				case KEY_A1:
					return Key::UpLeft;

				case KEY_A3:
					return Key::UpRight;

				case KEY_C1:
					return Key::DownLeft;

				case KEY_C3:
					return Key::DownRight;

				case ' ':
					return Key::Space;

				case KEY_ENTER:
				case '\n':
				case '\r':
					return Key::Enter;

				case KEY_BACKSPACE:
				case '\b':
				case 127:
					return Key::Backspace;

				case 27:
					return Key::Escape;

				case KEY_RESIZE:
					return Key::Resize;

				default:
					if (raw_key >= 33 && raw_key <= 126)
						return Key::Alphanumeric;

					return Key::None;
			}
		}

		bool is_shift_modified(const int raw_key)
		{
			switch (raw_key)
			{
				case KEY_SLEFT:
				case KEY_SRIGHT:
					return true;

#ifdef KEY_SUP
				case KEY_SUP:
					return true;
#endif

#ifdef KEY_SDOWN
				case KEY_SDOWN:
					return true;
#endif

				default:
					return false;
			}
		}

		char translate_character(const int raw_key)
		{
			if (raw_key < 0 || raw_key > UCHAR_MAX)
				return '\0';

			return static_cast<char>(raw_key);
		}
	}
	Vec2<int> to_direction(const Key key)
	{
		switch (key)
		{
			case Key::Up:
				return Vec2<int>{-1, 0};

			case Key::UpLeft:
				return Vec2<int>{-1, -1};

			case Key::UpRight:
				return Vec2<int>{-1, 1};

			case Key::Down:
				return Vec2<int>{1, 0};

			case Key::DownLeft:
				return Vec2<int>{1, -1};

			case Key::DownRight:
				return Vec2<int>{1, 1};

			case Key::Left:
				return Vec2<int>{0, -1};

			case Key::Right:
				return Vec2<int>{0, 1};

			default:
				Error::fatal("Ncurses::Input::to_direction(key) called for non-directional key");
		}
	}
	Event get_event(const int timeout_ms)
	{
		// ncurses already provides exactly the required behavior:
		//   timeout_ms < 0: blocking
		//   timeout_ms = 0: non-blocking
		//   timeout_ms > 0: wait this many milliseconds
		::timeout(timeout_ms);

		const int raw_key = getch();

		if (raw_key == ERR)
			return Event{};

		return Event{
			.key = translate_key(raw_key),
				.ch = translate_character(raw_key),
				.shift = is_shift_modified(raw_key),
		};
	}
}
