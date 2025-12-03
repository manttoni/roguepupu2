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
	wchar_t glyph;
	Color color;
};

struct Glow
{
	double strength;
	double radius;
};

struct Vision
{
	int range;
};

struct Inventory
{
	std::vector<entt::entity> inventory;
};

struct Solid {};
struct Opaque {};
struct Player {};

