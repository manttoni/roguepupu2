#pragma once

#include "Color.hpp"

struct Liquid
{
	enum class Type
	{
		Water,
		Blood,
	};
	Type type;

	bool operator<(const Liquid& other) const
	{
		return type < other.type;
	}
};
