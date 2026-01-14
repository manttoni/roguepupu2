#pragma once

struct Vec2
{
	int y, x;
	Vec2(const int y, const int x) : y(y), x(x) {}

	bool operator!=(const Vec2& other) const = default;
	bool operator==(const Vec2& other) const = default;
};

