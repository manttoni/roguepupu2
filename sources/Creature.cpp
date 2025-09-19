#include <string>
#include "Creature.hpp"

Creature::Creature() {}
Creature::Creature(const std::string& name, const short color_id, const char ch, Cell* cell) : Entity(name, color_id, ch, cell)
{
}
