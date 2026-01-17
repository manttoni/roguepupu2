#pragma once

#include <cassert>

struct Vec2
{
	int y, x;
	Vec2() : y(0), x(0) {}
	Vec2(const int y, const int x) : y(y), x(x) {}
	explicit Vec2(const size_t idx, const size_t size) :
		y(static_cast<int>(idx / size)),
		x(static_cast<int>(idx % size))
	{}
	Vec2(const Vec2& other) = default;

	bool operator!=(const Vec2& other) const = default;
	bool operator==(const Vec2& other) const = default;

	Vec2& operator=(const Vec2& other) = default;

	Vec2 operator+(const Vec2& other) const
	{
		return { y + other.y, x + other.x };
	}
	Vec2 operator-(const Vec2& other) const
	{
		return { y - other.y, x - other.x };
	}
	Vec2 operator/(const int s) const
	{
		assert(s != 0);
		return { y / s, x / s };
	}
	Vec2 operator*(const int s) const
	{
		return { y * s, x * s };
	}
};

