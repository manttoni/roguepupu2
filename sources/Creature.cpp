#include <string>
#include "Creature.hpp"

Creature::Creature() : vision_range(10) {}
Creature::Creature(const std::string& name, const short color_id, const char ch, Cell* cell) : Entity(name, color_id, ch, cell), vision_range(10)
{
}
