#include <format>
#include "systems/ExamineSystem.hpp"
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

	void show_entities_info(entt::registry& registry, std::vector<entt::entity>& entities)
	{
		if (entities.empty())
			return;
		std::vector<std::string> options;
		options.push_back("Next");
		const auto player = *registry.view<Player>().begin();
		Cell* player_cell = ECS::get_cell(registry, player);
		Cell* entity_cell = ECS::get_cell(registry, entities[0]); // all entities are in the same cell so any will work
		if (player_cell->get_cave()->distance(*player_cell, *entity_cell) <= MELEE_RANGE)
			options.push_back("Pick up");
		options.push_back("Cancel");

		auto it = entities.begin();
		while (!entities.empty())
		{
			auto valid_options = options;
			if (entities.size() == 1)
			{
				auto it_next = std::find(valid_options.begin(), valid_options.end(), "Next");
				if (it_next != valid_options.end())
					valid_options.erase(it_next);
			}

			const auto& category = ECS::get_category(registry, *it);
			if (category != "items" && ECS::get_name(registry, *it) != "glowing mushroom")
			{
				auto it_pick = std::find(valid_options.begin(), valid_options.end(), "Pick up");
				if (it_pick != valid_options.end())
					valid_options.erase(it_pick);
			}

			assert(std::find(valid_options.begin(), valid_options.end(), "Cancel") != valid_options.end());

			const auto& info_neat = get_info_neat(registry, *it);
			const auto& selected = UI::instance().dialog(info_neat, valid_options, Screen::topleft());
			if (selected == "Next")
			{
				if (++it == entities.end())
					it = entities.begin();
			}
			else if (selected == "Pick up")
			{
				pick_up(registry, player, *it);
				it = entities.erase(it);
				player_cell->get_cave()->draw();
			}
			else if (selected == "Cancel" || selected.empty())
				break;
		}
	}
};
