#include <string>
#include "Creature.hpp"

Creature::Creature() : Entity("default", 0) {}
Creature::Creature(const std::string& name) : Entity(name, 0) {}
