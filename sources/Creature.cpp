#include <string>
#include "Creature.hpp"

Creature::Creature() {}
Creature::Creature(const std::string& name) : Entity(name, 0) {}
