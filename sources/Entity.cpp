#include <string>
#include "Entity.hpp"
#include "Utils.hpp"
#include "Cell.hpp"

Entity::Entity() :
	name("default"),
	ch('?')
{}
Entity::Entity(const std::string& name, const short color_pair_id, const char ch, Cell* cell) : name(name), color_pair_id(color_pair_id), ch(ch), cell(cell)
{
	if (ch == '\0')
		this->ch = name.front();
}

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

size_t Entity::get_idx() const
{
	return cell->get_idx();
}
