#include <format>
#include "systems/ExamineSystem.hpp"
#include "systems/InventorySystem.hpp"
#include "UI.hpp"
#include "Utils.hpp"
#include "ECS.hpp"

namespace ExamineSystem
{
	std::vector<entt::entity> get_clicked_entities(Cave& cave, const size_t player_idx)
	{
		Screen::Coord mouse_position = UI::instance().get_mouse_position();
		PANEL* panel = UI::instance().get_panel(UI::Panel::GAME);
		WINDOW* window = panel_window(panel);
		int window_height, window_width, window_starty, window_startx;
		getmaxyx(window, window_height, window_width);
		getbegyx(window, window_starty, window_startx);

		int mouse_y = mouse_position.y - window_starty;
		int mouse_x = mouse_position.x - window_startx;

		int window_center_y = window_height / 2;
		int window_center_x = window_width / 2;

		int offset_y = mouse_y - window_center_y;
		int offset_x = mouse_x - window_center_x;

		int player_y = player_idx / cave.get_width();
		int player_x = player_idx % cave.get_width();

		int dest_y = player_y + offset_y;
		int dest_x = player_x + offset_x;
		const size_t dest_idx = dest_y * cave.get_width() + dest_x;

		auto entities = cave.get_cell(dest_idx).get_entities();
		return entities;
	}

	std::vector<std::string> get_info_neat(const entt::registry& registry, const entt::entity entity)
	{
		const auto& info = ECS::get_info(registry, entity);
		size_t leftcol = 0;
		size_t rightcol = 0;
		for (const auto& [left, right] : info)
		{
			leftcol = std::max(leftcol, left.size());
			rightcol = std::max(rightcol, right.size());
		}

		std::vector<std::string> info_neat = { ECS::get_colored_name(registry, entity) };
		for (const auto& [left, right] : info)
			info_neat.push_back(std::format("{: <{}} : {: >{}}", left, leftcol, right, rightcol));
		return info_neat;
	}

	void add_item(entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		auto& inventory = registry.get<Inventory>(entity).inventory;
		inventory.push_back(item);
	}

	void pick_up(entt::registry& registry, const entt::entity picker, const entt::entity entity)
	{
		registry.remove<Position>(entity);
		add_item(registry, picker, entity);

		if (registry.all_of<Glow>(entity))
			ECS::get_cell(registry, picker)->get_cave()->reset_lights();
	}

	bool can_take(const entt::registry& registry, const entt::entity entity, const entt::entity item)
	{
		const auto& category = registry.get<Category>(item).category;
		const auto& name = registry.get<Name>(item).name;
		if (category != "items" && name != "glowing mushroom")
			return false;

		Cell* entity_cell = ECS::get_cell(registry, entity);
		Cell* item_cell = ECS::get_cell(registry, item);
		if (entity_cell->get_cave()->distance(*entity_cell, *item_cell) > MELEE_RANGE)
			return false;

		return true;
	}

	bool can_open(const entt::registry& registry, const entt::entity entity, const entt::entity container)
	{
		if (!registry.all_of<Inventory>(container))
			return false;
		if (ECS::get_cell(registry, entity) != ECS::get_cell(registry, entity))
			return false;
		if (ECS::distance(registry, entity, container) > MELEE_RANGE)
			return false;
		return true;
	}

	void show_entities_info(entt::registry& registry, std::vector<entt::entity>& entities)
	{
		if (entities.empty())
			return;
		const auto player = *registry.view<Player>().begin();
		Cell* player_cell = ECS::get_cell(registry, player);
		auto it = entities.begin();
		while (!entities.empty())
		{
			std::vector<std::string> options;
			if (can_take(registry, player, *it))
				options.push_back("Take");
			if (can_open(registry, player, *it))
				options.push_back("Open");
			if (entities.size() > 1)
				options.push_back("Next");
			options.push_back("Cancel");

			const auto& info_neat = get_info_neat(registry, *it);
			const auto& selected = UI::instance().dialog(info_neat, options, Screen::topleft());
			if (selected == "Next")
			{
				if (++it == entities.end())
					it = entities.begin();
			}
			else if (selected == "Take")
			{
				pick_up(registry, player, *it);
				it = entities.erase(it);
				player_cell->get_cave()->draw();
			}
			else if (selected == "Open")
			{
				InventorySystem::open_inventory(registry, *it);
			}
			else if (selected == "Cancel" || selected.empty())
				break;
		}
	}
};
