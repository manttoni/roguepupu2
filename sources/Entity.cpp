#include <string>
#include "Entity.hpp"
#include "Utils.hpp"
#include "Cell.hpp"
#include "Cave.hpp"
#include "UI.hpp"

Entity::Entity() :
	name("default"),
	ch('?')
{}
Entity::Entity(const std::string& name, const short color_pair_id, const char ch, Cell* cell) : name(name), color_pair_id(color_pair_id), ch(ch), cell(cell)
{
	if (ch == '\0')
		this->ch = name.front();
}
bool Entity::operator==(const Entity& other)
{
	return name == other.name;
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

double Entity::move(const Direction d)
{
	//Log::log(name + " is moving");
	auto* cave = cell->get_cave();
	const auto& neighbor_ids = cave->get_nearby_ids(get_idx(), 1.5);
	int dst = get_idx();
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
	if (dst < 0 || dst >= static_cast<int>(cave->get_size()))
		return 0;
	if (!Utils::contains(neighbor_ids, static_cast<size_t>(dst)))
		return 0;

	auto& new_cell = cave->get_cells()[dst];
	if (new_cell.blocks_movement())
		return 0;

	// find own ponter
	std::unique_ptr<Entity> ent_ptr;
	for (std::unique_ptr<Entity>& entity : cave->get_cells()[get_idx()].get_entities())
		if (*entity == *this)
			ent_ptr = std::move(entity);

	const auto& old_cell = *cell;
	ent_ptr->set_cell(&new_cell);
	new_cell.add_entity(std::move(ent_ptr));

	// add debug info
	if (name == "Rabdin")
	{
		auto& debug = UI::instance().get_menu("debug");
		debug.set_value("player_y", new_cell.get_idx() / cave->get_width());
		debug.set_value("player_x", new_cell.get_idx() % cave->get_width());
	}

	return cave->distance(old_cell, new_cell);
}
