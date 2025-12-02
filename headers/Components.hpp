#pragma once

#include <string>
#include "Cell.hpp"
#include "Color.hpp"

struct Name
{
	std::string name;
};

struct Position
{
	Cell* cell;
};

struct Renderable
{
	wchar_t symbol;
	Color color;
};

struct Glow
{
	Color color;
	double intensity;
	double radius;
};

struct Vision
{
	int range;
};

struct Solid {};
struct Opaque {};
struct Player {};

