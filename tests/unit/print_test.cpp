#include <gtest/gtest.h>
#include <curses.h>
#include <string>

#include "ncurses/Color.hpp"
#include "components/Component.hpp"
#include "gtest/gtest.h"
#include "ncurses/Attribute.hpp"

TEST(PrintTest, ColorMarkup)
{
	Ncurses::Color color(123,123,123);
	const auto markup = color.markup();
	EXPECT_EQ(markup, "{123,123,123}");
}

TEST(PrintTest, IsColorMarkupWorks)
{
	const std::string markup = "{123,123,123}";
	EXPECT_TRUE(Ncurses::Color::is_markup(markup, 0)) << markup;

	const std::string with_space = " {123,123,123}";
	EXPECT_TRUE(Ncurses::Color::is_markup(with_space, 1)) << with_space << "[1]";
	EXPECT_FALSE(Ncurses::Color::is_markup(with_space, 0)) << with_space << "[0]";

	const std::string wrong_markup = "123,123,123}";
	EXPECT_FALSE(Ncurses::Color::is_markup(wrong_markup, 0)) << wrong_markup;
}

TEST(PrintTest, FromColorMarkupWorks)
{
	const std::string markup = "{123,123,123}";
	const Ncurses::Color color = Ncurses::Color::from_markup(markup, 0);
	EXPECT_EQ(color, Ncurses::Color(123,123,123)) << markup;
}

TEST(PrintTest, AttrMarkup)
{
	Ncurses::Attribute attr(A_DIM);
	const auto markup = attr.markup();
	EXPECT_EQ(markup, "[A_DIM]");
}

TEST(PrintTest, IsAttrMarkupWorks)
{
	const std::string markup = "[A_BOLD]";
	EXPECT_TRUE(Ncurses::Attribute::is_markup(markup, 0)) << markup;

	const std::string with_space = " [A_BOLD]";
	EXPECT_TRUE(Ncurses::Attribute::is_markup(with_space, 1)) << with_space << "[1]";
	EXPECT_FALSE(Ncurses::Attribute::is_markup(with_space, 0)) << with_space << "[0]";

	const std::string wrong_markup = "A_BOLD]";
	EXPECT_FALSE(Ncurses::Attribute::is_markup(wrong_markup, 0)) << wrong_markup;
}

TEST(PrintTest, FromAttrMarkupWorks)
{
	const std::string markup = "[A_DIM]";
	const chtype attr = Ncurses::Attribute::from_markup(markup, 0);
	EXPECT_EQ(attr, A_DIM);
}
