#include <curses.h>
#include <stddef.h>
#include <algorithm>  // for min, max
#include <string>     // for operator+, allocator, char_traits, to_string
#include <regex>
#include <stdexcept>

#include "domain/Color.hpp"  // for Color
#include "UI/UI.hpp"     // for UI

Color::Color(const short r, const short g, const short b) :
	r(std::min<short>(1000, std::max<short>(r, 0))),
	g(std::min<short>(1000, std::max<short>(g, 0))),
	b(std::min<short>(1000, std::max<short>(b, 0)))
{}

bool Color::operator==(const Color& other) const
{
	return	r == other.r &&
		g == other.g &&
		b == other.b;
}
bool Color::operator!=(const Color& other) const
{
	return	r != other.r ||
		g != other.g ||
		b != other.b;
}
bool Color::operator<(const Color& other) const
{
	if (r != other.r)
		return r < other.r;
	if (g != other.g)
		return g < other.g;
	return b < other.b;
}
Color& Color::operator+=(const Color& other)
{
	r = std::min(r + other.r, 1000);
	g = std::min(g + other.g, 1000);
	b = std::min(b + other.b, 1000);
	return *this;
}
Color Color::operator*(const int scalar) const
{
	return Color(
			static_cast<short>(std::min(r * scalar, 1000)),
			static_cast<short>(std::min(g * scalar, 1000)),
			static_cast<short>(std::min(b * scalar, 1000)));
}
Color Color::operator*(const double scalar) const
{
	return Color(
			static_cast<short>(std::min(r * scalar, 1000.0)),
			static_cast<short>(std::min(g * scalar, 1000.0)),
			static_cast<short>(std::min(b * scalar, 1000.0)));
}
Color Color::operator/(const int scalar) const
{
	return Color(
			static_cast<short>(std::min(r / scalar, 1000)),
			static_cast<short>(std::min(g / scalar, 1000)),
			static_cast<short>(std::min(b / scalar, 1000)));
}
Color Color::operator/(const double scalar) const
{
	return Color(
			static_cast<short>(std::min(r / scalar, 1000.0)),
			static_cast<short>(std::min(g / scalar, 1000.0)),
			static_cast<short>(std::min(b / scalar, 1000.0)));
}

short Color::init() const
{
	short color_id = UI::instance().is_initialized_color(*this);
	if (color_id != -1)
		return color_id;
	static short color_id_counter = 8;
	color_id = color_id_counter++;
	if (color_id_counter >= 256)
		color_id_counter = 8;
	init_color(color_id, r, g, b); // from ncurses
	UI::instance().set_initialized_color(*this, color_id);
	return color_id;
}

std::string Color::markup() const
{
	return "{" + std::to_string(r) + "," + std::to_string(g) + "," + std::to_string(b) + "}";
}

double Color::get_illumination() const
{
	const size_t channels_sum = r + g + b;
	return static_cast<double>(channels_sum) / 3000.0;
}

bool Color::is_markup(const std::string& str, const size_t idx)
{
	if (str[idx] != '{')
		return false;

	const auto close = str.find('}', idx);
	if (close == std::string::npos)
		return false;

	const auto markup = str.substr(idx, close - idx + 1);
	if (markup == "{reset}")
		return true;
	std::regex regex(R"(\{(\d+),(\d+),(\d+)\})");
	std::smatch match;
	if (!std::regex_match(markup, match, regex))
		return false;


	const int r = std::stoi(match[1].str());
	const int g = std::stoi(match[2].str());
	const int b = std::stoi(match[3].str());

	if (r < 0 || r > 1000 ||
			g < 0 || g > 1000 ||
			b < 0 || b > 1000)
		return false;
	return true;

}

Color Color::from_markup(const std::string& str, const size_t idx)
{
	const auto close = str.find('}', idx);

	const auto markup = str.substr(idx, close - idx + 1);
	std::regex regex(R"(\{(\d+),(\d+),(\d+)\})");
	std::smatch match;
	if (!std::regex_match(markup, match, regex))
		throw std::runtime_error("Invalid color markup: " + markup);

	const int r = std::stoi(match[1].str());
	const int g = std::stoi(match[2].str());
	const int b = std::stoi(match[3].str());

	return Color(r, g, b);
}
