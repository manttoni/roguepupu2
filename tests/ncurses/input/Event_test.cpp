#include <gtest/gtest.h>

#include <climits>
#include <stdexcept>

#include <ncurses.h>

#include "ncurses/Input.hpp"

namespace
{
	namespace Input = Ncurses::Input;
	using Key = Input::Key;

	TEST(InputTranslation, MapsSpecialKeys)
	{
		struct Case
		{
			int raw;
			Key expected;
		};

		const Case cases[] = {
			{ERR,           Key::None},
			{KEY_UP,        Key::Up},
			{KEY_DOWN,      Key::Down},
			{KEY_LEFT,      Key::Left},
			{KEY_RIGHT,     Key::Right},
			{KEY_SLEFT,     Key::Left},
			{KEY_SRIGHT,    Key::Right},
			{KEY_A1,        Key::UpLeft},
			{KEY_A3,        Key::UpRight},
			{KEY_C1,        Key::DownLeft},
			{KEY_C3,        Key::DownRight},
			{' ',           Key::Space},
			{KEY_ENTER,     Key::Enter},
			{'\n',          Key::Enter},
			{'\r',          Key::Enter},
			{KEY_BACKSPACE, Key::Backspace},
			{'\b',          Key::Backspace},
			{127,           Key::Backspace},
			{27,            Key::Escape},
			{KEY_RESIZE,    Key::Resize},
		};

		for (const auto& test : cases)
		{
			SCOPED_TRACE(test.raw);
			EXPECT_EQ(Input::translate_key(test.raw), test.expected);
		}
	}

	TEST(InputTranslation, ClassifiesEveryPrintableNonSpaceAsciiCharacter)
	{
		const std::string digits = "0123456789";
		const std::string letters =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";

		for (int raw = 33; raw <= 126; ++raw)
		{
			SCOPED_TRACE(raw);
			const char ch = static_cast<char>(raw);

			Key expected = Key::Symbol;

			if (digits.find(ch) != std::string::npos)
				expected = Key::Number;
			else if (letters.find(ch) != std::string::npos)
				expected = Key::Alphabetic;

			EXPECT_EQ(Input::translate_key(raw), expected);
		}
	}

	TEST(InputTranslation, MapsUnhandledInputToNone)
	{
		for (const int raw : {0, 1, 31, 128})
		{
			SCOPED_TRACE(raw);
			EXPECT_EQ(Input::translate_key(raw), Key::None);
		}
	}

	TEST(InputCharacter, PreservesPrintableAscii)
	{
		for (int raw = 32; raw <= 126; ++raw)
		{
			SCOPED_TRACE(raw);

			EXPECT_EQ(
					Input::translate_character(raw),
					static_cast<char>(raw));
		}
	}

	TEST(InputCharacter, PreservesControlCharacterBytes)
	{
		for (const char ch : {'\0', '\b', '\n', '\r'})
		{
			SCOPED_TRACE(static_cast<int>(ch));
			EXPECT_EQ(Input::translate_character(ch), ch);
		}
	}

	TEST(InputCharacter, HandlesByteRangeBoundaries)
	{
		EXPECT_EQ(Input::translate_character(-1), '\0');
		EXPECT_EQ(Input::translate_character(0), '\0');

		EXPECT_EQ(
				Input::translate_character(UCHAR_MAX),
				static_cast<char>(UCHAR_MAX));

		EXPECT_EQ(Input::translate_character(UCHAR_MAX + 1), '\0');
		EXPECT_EQ(Input::translate_character(INT_MAX), '\0');
	}

	TEST(InputCharacter, DoesNotTurnSpecialKeysIntoCharacters)
	{
		for (const int raw : {
				KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
				KEY_ENTER, KEY_BACKSPACE, KEY_RESIZE
			})
		{
			SCOPED_TRACE(raw);
			EXPECT_EQ(Input::translate_character(raw), '\0');
		}
	}

	TEST(InputModifiers, RecognizesShiftedArrowCodes)
	{
		EXPECT_TRUE(Input::is_shift_modified(KEY_SLEFT));
		EXPECT_TRUE(Input::is_shift_modified(KEY_SRIGHT));

#ifdef KEY_SUP
		EXPECT_TRUE(Input::is_shift_modified(KEY_SUP));
#endif

#ifdef KEY_SDOWN
		EXPECT_TRUE(Input::is_shift_modified(KEY_SDOWN));
#endif
	}

	TEST(InputModifiers, DoesNotMarkOrdinaryKeysAsShiftModified)
	{
		for (const int raw : {
				ERR, KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN,
				static_cast<int>('a'),
				static_cast<int>('A'),
				static_cast<int>('!'),
				static_cast<int>(' ')
			})
		{
			SCOPED_TRACE(raw);
			EXPECT_FALSE(Input::is_shift_modified(raw));
		}
	}

	TEST(InputEvent, ConvertsEveryDirectionToAnOffset)
	{
		struct Case
		{
			Key key;
			int y;
			int x;
		};

		const Case cases[] = {
			{Key::Up,        -1,  0},
			{Key::UpLeft,    -1, -1},
			{Key::UpRight,   -1,  1},
			{Key::Down,       1,  0},
			{Key::DownLeft,   1, -1},
			{Key::DownRight,  1,  1},
			{Key::Left,       0, -1},
			{Key::Right,      0,  1},
		};

		for (const auto& test : cases)
		{
			SCOPED_TRACE(static_cast<int>(test.key));
			const Input::Event event{.key = test.key};

			EXPECT_TRUE(event.is_directional());

			const auto direction = event.to_direction();
			EXPECT_EQ(direction.y, test.y);
			EXPECT_EQ(direction.x, test.x);
		}
	}

	TEST(InputEvent, RejectsNonDirectionalKeysAsDirections)
	{
		for (const Key key : {
				Key::None,
				Key::Alphabetic,
				Key::Number,
				Key::Symbol,
				Key::Space,
				Key::Enter,
				Key::Backspace,
				Key::Escape,
				Key::Resize
			})
		{
			SCOPED_TRACE(static_cast<int>(key));
			const Input::Event event{.key = key};

			EXPECT_FALSE(event.is_directional());
			EXPECT_THROW(event.to_direction(), std::runtime_error);
		}
	}

	TEST(InputEvent, DefaultsToNoInput)
	{
		const Input::Event event{};

		EXPECT_EQ(event.key, Key::None);
		EXPECT_EQ(event.ch, '\0');
		EXPECT_FALSE(event.shift);
		EXPECT_FALSE(event.is_directional());
	}
}
