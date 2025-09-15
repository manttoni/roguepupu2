#include <string>
#include "Entity.hpp"
#include "Utils.hpp"

Entity::Entity() :
	name("default"),
	ch('?')
{}
Entity::Entity(const std::string& name, const short color_pair_id) :
	name(name),
	color_pair_id(color_pair_id),
	ch(name.front())
{
	Log::log("Entity created: " + name + " " + std::to_string(color_pair_id));
}


