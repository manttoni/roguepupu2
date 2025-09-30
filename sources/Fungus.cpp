#include <string>
#include <map>
#include "Fungus.hpp"
#include "UI.hpp"

Fungus::Fungus(
		const std::string& name,
		const wchar_t wchar,
		const Color& color,
		const std::vector<Light>& lights)
	: Entity(Entity::Type::FUNGUS, name, wchar, color, lights) {}
Fungus::~Fungus() {}
Fungus::Fungus(const Fungus& other) : Entity(Entity::Type::FUNGUS, other.name, other.symbol, other.color, other.lights)
{
	set_cell(other.cell);
}
Fungus& Fungus::operator=(const Fungus& other)
{
	if (this != &other)
	{
		name = other.name;
		symbol = other.symbol;
		color = other.color;
		cell = other.cell;
		lights = other.lights;
	}
	return *this;
}
