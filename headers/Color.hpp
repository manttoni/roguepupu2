#pragma once

#include <cstddef>

class Color
{
	private:
		short r, g, b;

	public:
		short get_r() const { return r; }
		short get_g() const { return g; }
		short get_b() const { return b; }

	public:
		Color();
		Color(const short r, const short g, const short b);
		Color(const Color& other);
		Color& operator=(const Color& other);
		bool operator==(const Color& other) const;
		bool operator!=(const Color& other) const;
		bool operator<(const Color& other) const;
		Color& operator+=(const Color& other);
		Color operator*(const int scalar) const;
		Color operator/(const int scalar) const;

		short init() const;
};
