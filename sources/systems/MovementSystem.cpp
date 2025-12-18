#include <curses.h>                    // for KEY_DOWN, KEY_LEFT, KEY_RIGHT
#include "ECS.hpp"
#include "Cave.hpp"                    // for Cave
#include "Cell.hpp"                    // for Cell
#include "Components.hpp"              // for Position
#include "Utils.hpp"                   // for Vec2
#include "entt.hpp"                    // for size_t, map, entity, registry
#include "systems/MovementSystem.hpp"  // for move, movement_key_pressed
#include "systems/CombatSystem.hpp"

namespace MovementSystem
{
	std::map<int, Vec2> movement_keys = {
		{KEY_UP,	{-1, 0}},
		{'w',		{-1, 0}},
		{KEY_RIGHT,	{0, 1}},
		{'d',		{0, 1}},
		{KEY_DOWN,	{1, 0}},
		{'s',		{1, 0}},
		{KEY_LEFT,	{0, -1}},
		{'a',		{0, -1}},
		{'7',		{-1, -1}},
		{'8',		{-1, 0}},
		{'9',		{-1, 1}},
		{'4',		{0, -1}},
		{'6',		{0, 1}},
		{'1',		{1, -1}},
		{'2',		{1, 0}},
		{'3',		{1, 1}}
	};

	bool movement_key_pressed(const int key)
	{
		return movement_keys.find(key) != movement_keys.end();
	}

	double move(entt::registry& registry, entt::entity entity, const int key)
	{
		return move(registry, entity, movement_keys[key]);
	}

	// Move to a direction if possible
	double move(entt::registry& registry, entt::entity entity, const Vec2 direction)
	{
		auto& position = registry.get<Position>(entity);
		auto* current_cell = position.cell;
		auto* current_cave = current_cell->get_cave();

		const size_t width = current_cave->get_width();
		const size_t src_idx = current_cell->get_idx();
		const size_t y = src_idx / width;
		const size_t x = src_idx % width;
		const size_t dst_idx = (direction.dy + y) * width + direction.dx + x;
		Cell* dst = &current_cave->get_cell(dst_idx);
		const auto& entities = dst->get_entities();
		for (auto& e : entities)
		{
			if (ECS::are_enemies(registry, entity, e))
			{
				CombatSystem::attack(registry, entity, e);
				return 0;
			}
		}
		if (!current_cave->has_access(src_idx, dst_idx))
			return 0;
		position.cell = &current_cave->get_cell(dst_idx);
		if (registry.all_of<Actions>(entity))
			registry.get<Actions>(entity).used++;
		return current_cave->distance(src_idx, dst_idx);
	}

	// Move to a cell even if there is no path
	// Can be used f.e. when changing level or teleporting
	double move(entt::registry& registry, entt::entity entity, Cell& new_cell)
	{
		if (new_cell.blocks_movement())
			return 0;
		auto& position = registry.get<Position>(entity);
		auto* current_cell = position.cell;
		auto* current_cave = current_cell->get_cave();
		auto* new_cave = new_cell.get_cave();

		position.cell = &new_cell;
		if (new_cave == current_cave)
			return new_cave->distance(*current_cell, new_cell);
		return 0;
	}
};
