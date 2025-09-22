#include <string>
#include "Entity.hpp"
#include "Utils.hpp"
#include "Cell.hpp"
#include "Cave.hpp"
#include "UI.hpp"

Entity::Entity() : name("default"), color(Color()), ch('?'), cell(nullptr) {}
Entity::Entity(const std::string& name, const Color& color, const char ch, Cell* cell) : name(name), color(color), ch(ch), cell(cell)
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
	auto* cave = cell->get_cave();
	assert(cell != nullptr);
	assert(cave != nullptr);
	assert(cave->get_size() > 0);
	assert(get_idx() < cave->get_size());
	const auto& neighbor_ids = cave->get_nearby_ids(get_idx(), 1.5);
	size_t dst = get_idx();
	Log::log("entity dst idx: " + std::to_string(dst));
	Log::log("cave size is: " + std::to_string(cave->get_size()));
	assert(neighbor_ids.size() >= 3 && neighbor_ids.size() <= 8);
	assert(dst < cave->get_size());
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
	assert(static_cast<size_t>(dst) < cave->get_size());
	if (dst >= cave->get_size())
		return 0;
	if (!Utils::contains(neighbor_ids, dst))
		return 0;

	auto& new_cell = cave->get_cells()[dst];
	if (new_cell.blocks_movement())
		return 0;


	Log::log(name + " is moving");
	// find own ponter
	std::unique_ptr<Entity> ent_ptr;
	for (std::unique_ptr<Entity>& entity : cave->get_cells()[get_idx()].get_entities())
		if (*entity == *this)
			ent_ptr = std::move(entity);

	auto& old_cell = *cell;
	ent_ptr->set_cell(&new_cell);
	new_cell.add_entity(std::move(ent_ptr));

	// add debug info
	if (name == "Rabdin")
	{
		auto& debug = UI::instance().get_menu("debug");
		debug.set_value("player_y", new_cell.get_idx() / cave->get_width());
		debug.set_value("player_x", new_cell.get_idx() % cave->get_width());
	}

	// remove empty unique ptr from old cell
	auto& old_entities = old_cell.get_entities();
	for (size_t i = 0; i < old_entities.size(); ++i)
	{
		auto& entity = old_entities[i];
		if (!entity)
		{
			old_entities.erase(old_entities.begin() + i);
			break;
		}
	}

	return cave->distance(old_cell, new_cell);
}
