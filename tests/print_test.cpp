#include <gtest/gtest.h>
#include "helpers.hpp"
#include "domain/Color.hpp"

TEST(PrintTest, ColorMarkup)
{
	Color color(123,123,123);
	const auto markup = color.markup();
	EXPECT_EQ(markup, "{123,123,123}");
}

TEST(PrintTest, IsColorMarkupWorks)
{
	const std::string markup = "{123,123,123}";
	EXPECT_TRUE(Color::is_markup(markup, 0)) << markup;

	const std::string with_space = " {123,123,123}";
	EXPECT_TRUE(Color::is_markup(with_space, 1)) << with_space << "[1]";
	EXPECT_FALSE(Color::is_markup(with_space, 0)) << with_space << "[0]";

	const std::string wrong_markup = "123,123,123}";
	EXPECT_FALSE(Color::is_markup(wrong_markup, 0)) << wrong_markup;
}

TEST(PrintTest, FromColorMarkupWorks)
{
	const std::string markup = "{123,123,123}";
	const Color color = Color::from_markup(markup, 0);
	EXPECT_EQ(color, Color(123,123,123)) << markup;
}

TEST(PrintTest, AttrMarkup)
{
	NcursesAttr attr(A_DIM);
	const auto markup = attr.markup();
	EXPECT_EQ(markup, "[A_DIM]");
}

TEST(PrintTest, IsAttrMarkupWorks)
{
	const std::string markup = "[A_BOLD]";
	EXPECT_TRUE(NcursesAttr::is_markup(markup, 0)) << markup;

	const std::string with_space = " [A_BOLD]";
	EXPECT_TRUE(NcursesAttr::is_markup(with_space, 1)) << with_space << "[1]";
	EXPECT_FALSE(NcursesAttr::is_markup(with_space, 0)) << with_space << "[0]";

	const std::string wrong_markup = "A_BOLD]";
	EXPECT_FALSE(NcursesAttr::is_markup(wrong_markup, 0)) << wrong_markup;
}

TEST(PrintTest, FromAttrMarkupWorks)
{
	const std::string markup = "[A_DIM]";
	const chtype attr = NcursesAttr::from_markup(markup, 0);
	EXPECT_EQ(attr, A_DIM);
}
