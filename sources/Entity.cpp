#include <string>
#include "Entity.hpp"
#include "Utils.hpp"
#include "Cell.hpp"
#include "Cave.hpp"
#include "UI.hpp"

Entity::Entity()
	: type(Type::NONE), name("default"), symbol(L'?'), color(Color(500, 500, 500)), cell(nullptr)
{}
Entity::~Entity() {}
Entity::Entity(	const Type type,
				const std::string& name,
				const wchar_t symbol,
				const Color& color,
				std::vector<Light> lights)
	: type(type), name(name), symbol(symbol), color(color), lights(lights)
{}
Entity::Entity(const Entity& other) :
	type(other.type),
	name(other.name),
	symbol(other.symbol),
	color(other.color),
	cell(other.cell),
	lights(other.lights)
{}
Entity& Entity::operator=(const Entity& other)
{
	if (this != &other)
	{
		type = other.type;
		name = other.name;
		symbol = other.symbol;
		color = other.color;
		cell = other.cell;
		lights = other.lights;
	}
	return *this;
}

bool Entity::operator==(const Entity& other)
{
	return name == other.name;
}

bool Entity::blocks_movement() const
{
	if (symbol== L'$')
		return true;
	return false;
}

bool Entity::blocks_vision() const
{
	if (symbol == L'$')
		return true;
	return false;
}

size_t Entity::get_idx() const
{
	return cell->get_idx();
}

std::unique_ptr<Entity> Entity::extract_self()
{
	auto& old_entities = cell->get_entities();
	for (auto it = old_entities.begin(); it != old_entities.end(); ++it)
	{
		if (it->get() == this)
		{
			std::unique_ptr<Entity> self = std::move(*it);
			old_entities.erase(it);
			return self;
		}
	}
	throw std::runtime_error("Entity " + name + " can't find its own pointer");
}

double Entity::move(Cell& new_cell)
{
	// find own pointer(self)
	std::unique_ptr<Entity> self = extract_self();

	// move self to new cell
	auto& old_cell = *cell;
	self->set_cell(&new_cell);
	new_cell.add_entity(std::move(self));

	// return distance of movement
	return cell->get_cave()->distance(old_cell, new_cell);
}

/*
// currently not in use, maybe never will be
double Entity::move(const Direction d)
{
	// find the destination Cell
	auto* cave = cell->get_cave();
	const auto& neighbor_ids = cave->get_nearby_ids(get_idx(), 1.5);
	size_t dst = get_idx();
	switch(d)
	{
		case Direction::DOWN:
			dst += cave->get_width();
			break;
		case Direction::UP:
			dst -= cave->get_width();
			break;
		case Direction::LEFT:
			dst--;
			break;
		case Direction::RIGHT:
			dst++;
			break;
	}
	if (dst >= cave->get_size())
		return 0;
	if (!Utils::contains(neighbor_ids, dst))
		return 0;

	auto& new_cell = cave->get_cells()[dst];
	if (new_cell.blocks_movement())
		return 0;
	return move(new_cell);
}
*/

