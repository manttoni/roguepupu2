#include "Color.hpp"
#include "Utils.hpp"
#include "UI.hpp"

Color::Color() :
	r(0), g(0), b(0)
{} // does not init color
Color::Color(const short id, const short r, const short g, const short b) :
	id(id),
	r(std::min<short>(1000, std::max<short>(r, 0))),
	g(std::min<short>(1000, std::max<short>(g, 0))),
	b(std::min<short>(1000, std::max<short>(b, 0)))
{} // COLOR HAS TO BE INITIALIZED BEFORE
Color::Color(const Color& other) :
	id(other.id), r(other.r), g(other.g), b(other.b)
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

