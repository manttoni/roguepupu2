#include <string>
#include "Entity.hpp"

Entity::Entity() :
	name("default"),
	ch('?')
{}
Entity::Entity(const std::string& name) :
	name(name),
	ch(name.front())
{}
