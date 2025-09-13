#include <string>
#include "Creature.hpp"

Creature::Creature() : Entity("default") {}
Creature::Creature(const std::string& name) : Entity(name) {}
