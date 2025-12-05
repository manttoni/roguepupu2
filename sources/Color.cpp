#include <sstream>
#include "Color.hpp"
#include "Utils.hpp"
#include "UI.hpp"

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
	init_color(color_id, r, g, b);
	UI::instance().set_initialized_color(*this, color_id);
	return color_id;
}

std::string Color::to_string() const
{
	return "(" + std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b) + ")";
}
