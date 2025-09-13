#include <string>
#include "Entity.hpp"

Entity::Entity() : name("default") {}
Entity::Entity(const std::string& name) : name(name) {}
