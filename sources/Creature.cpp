#include <string>
#include "Creature.hpp"

Creature::Creature() : vision_range(10) {}
Creature::Creature(const std::string& name, const Color& color, const char ch, Cell* cell) : Entity(name, color, ch, cell), vision_range(10)
{
}
