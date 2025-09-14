#include "Color.hpp"
#include "Utils.hpp"
#include "UI.hpp"

Color::Color() :
	r(0), g(0), b(0)
{}
Color::Color(const short r, const short g, const short b) :
	r(std::min<short>(1000, std::max<short>(r, 0))),
	g(std::min<short>(1000, std::max<short>(g, 0))),
	b(std::min<short>(1000, std::max<short>(b, 0)))
{
	id = UI::instance().get_next_color_id();
	init_color(id, this->r, this->g, this->b);
}
Color::Color(const Color& other) :
	r(other.r), g(other.g), b(other.b), id(other.id)
{}
Color& Color::operator=(const Color& other)
{
	if (this == &other)
		return *this;

	r = other.r;
	g = other.g;
	b = other.b;
	id = other.id;
	return *this;
}
bool Color::operator==(const Color& other)
{
	return	r == other.r &&
			g == other.g &&
			b == other.b;
}
bool Color::operator!=(const Color& other)
{
	return	r != other.r ||
			g != other.g ||
			b != other.b;
}

Color Color::operator+(const Color& other) const
{
	return Color(r + other.r, g + other.g, b + other.b);
}
