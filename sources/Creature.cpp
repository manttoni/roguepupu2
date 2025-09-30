#include <string>
#include "Creature.hpp"

Creature::Creature() {}
Creature::~Creature() {}
Creature::Creature(
		const std::string& name,
		const wchar_t symbol,
		const Color& color,
		const double vision_range)
	: Entity(Entity::Type::CREATURE, name, symbol, color), vision_range(vision_range)
{}
Creature::Creature(const Creature& other) :
	Entity(Entity::Type::CREATURE, other.name, other.symbol, other.color), vision_range(other.vision_range)
{
	set_cell(other.cell);
}
Creature& Creature::operator=(const Creature& other)
{
	if (this != &other)
	{
		Entity::operator=(other);
		vision_range = other.vision_range;
	}
	return *this;
}
