#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "ui/Element.hpp"
#include "ui/Format.hpp"

namespace
{
	TEST(FormatWidthTest, CountsTheVisibleCursorOnce)
	{
		EXPECT_EQ(UI::Format::visible_width("[A_BLINK]|[reset]"), 1u);
	}

	TEST(FormatWidthTest, IgnoresBothKindsOfMarkupIncludingAdjacentResets)
	{
		EXPECT_EQ(UI::Format::visible_width("{1,2,3}[A_BOLD]HP[reset]{reset}"), 2u);
		EXPECT_EQ(UI::Format::visible_width("{1,2,3}{reset}[A_DIM][reset]"), 0u);
		EXPECT_EQ(UI::Format::visible_width(""), 0u);
	}

	TEST(FormatWidthTest, CountsLiteralBracketsAndIncompleteTags)
	{
		for (const std::string literal : {"[X]", "[ ]", "[A_UNKNOWN]", "[A_BLINK", "{1,2"})
		{
			SCOPED_TRACE(literal);
			EXPECT_EQ(UI::Format::visible_width(literal), literal.size());
		}
	}

	TEST(FormatElementsTest, AlignsStyledAndPlainLabelsByVisibleColumns)
	{
		const std::string value = "12";
		bool enabled = true;
		const std::vector<UI::Element::Any> elements{
			UI::Element::TextOut{.label = "{2,1,0}[A_BOLD]HP[reset]{reset}", .text = &value},
			UI::Element::Checkbox{.label = "Enabled", .value = &enabled},
			UI::Element::Button{.label = "[A_DIM]Cancel[reset]"}
		};

		const auto lines = UI::Format::elements(elements);

		ASSERT_EQ(lines.size(), 3u);
		EXPECT_EQ(lines[0], "{2,1,0}[A_BOLD]HP[reset]{reset}      : 12");
		EXPECT_EQ(lines[1], "Enabled : [X]");
		EXPECT_EQ(lines[2], "[A_DIM]Cancel[reset]");
		const auto first_colon = lines[0].find(':');
		const auto second_colon = lines[1].find(':');
		ASSERT_NE(first_colon, std::string::npos);
		ASSERT_NE(second_colon, std::string::npos);
		EXPECT_EQ(UI::Format::visible_width(lines[0].substr(0, first_colon)),
			UI::Format::visible_width(lines[1].substr(0, second_colon)));
	}

	TEST(FormatElementsTest, ValuesDoNotMoveTheLabelSeparator)
	{
		const std::string time = "12:34";
		const std::string name = "Bob";
		const auto lines = UI::Format::elements({
			UI::Element::TextOut{.label = "[A_BOLD]At[reset]", .text = &time},
			UI::Element::TextOut{.label = "Name", .text = &name}
		});

		ASSERT_EQ(lines.size(), 2u);
		EXPECT_EQ(lines[0], "[A_BOLD]At[reset]   : 12:34");
		EXPECT_EQ(lines[1], "Name : Bob");
	}

	TEST(TextInCursorTest, KeepsTheFieldWidthWhenTheCursorDisappearsAtCapacity)
	{
		std::string input = "abc";
		const UI::Element::TextIn field{.label = "", .text = &input, .length = {0, 6}};

		const auto partial = UI::Element::to_string(field);
		EXPECT_EQ(partial, "abc[A_BLINK]|[reset]  ");
		EXPECT_EQ(UI::Format::visible_width(partial), 6u);
		EXPECT_EQ(input, "abc");

		input = "abcdef";
		const auto full = UI::Element::to_string(field);
		EXPECT_EQ(full, "abcdef");
		EXPECT_EQ(UI::Format::visible_width(full), 6u);
	}
}
