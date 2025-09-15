#include <string>
#include "Entity.hpp"
#include "Utils.hpp"

Entity::Entity() :
	name("default"),
	ch('?')
{}
Entity::Entity(
		const std::string& name,
		const short color_pair_id) :
	name(name),
	color_pair_id(color_pair_id),
	ch(name.front())
{}

bool Entity::blocks_movement() const
{
	if (ch == '$')
		return true;
	return false;
}

bool Entity::blocks_vision() const
{
	if (ch == '$')
		return true;
	return false;
}
