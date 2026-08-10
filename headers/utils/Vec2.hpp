#pragma once

#include <cassert>
#include <cmath>
#include <sstream>
#include <ostream>

template <typename T>
struct Vec2
{
	T y, x;
	Vec2() : y{}, x{} {}
	Vec2(const T y, const T x) : y(y), x(x) {}
	Vec2(const Vec2& other) = default;
	template <typename U>
		explicit operator Vec2<U>() const
		{
			return {
				static_cast<U>(y),
					static_cast<U>(x)
			};
		}
	double length() const
	{
		return std::hypot(y, x);
	}

	Vec2 normalize() const
	{
		const auto len = length();
		if (len == T{})
			return *this;
		return *this / len;
	}

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
	Vec2 operator/(const T s) const
	{
		assert(s != T{});
		return { y / s, x / s };
	}
	Vec2 operator*(const T s) const
	{
		return { y * s, x * s };
	}
	Vec2& operator+=(const Vec2& other)
	{
		y += other.y;
		x += other.x;
		return *this;
	}
	Vec2& operator-=(const Vec2& other)
	{
		y -= other.y;
		x -=other.x;
		return *this;
	}
	Vec2 round() const
	{
		return {
			static_cast<T>(std::round(y)),
				static_cast<T>(std::round(x))
		};
	}

	// Convert 'this' to a cell_idx
	size_t to_idx(const size_t cave_size) const
	{
		assert(y >= 0 && x >= 0 &&
				static_cast<size_t>(y) < cave_size &&
				static_cast<size_t>(x) < cave_size);
		return static_cast<size_t>(y) * cave_size + static_cast<size_t>(x);
	}

	static inline Vec2<T>from_idx(const size_t idx, const size_t cave_size)
	{
		return Vec2<T>(idx / cave_size, idx % cave_size);
	}

	std::string to_string() const
	{
		std::ostringstream ss;
		ss << "y:" << y << ",x:" << x;
		return ss.str();
	}

	friend std::ostream& operator<<(std::ostream& os, const Vec2& v)
	{
		os << v.to_string();
		return os;
	}
};

