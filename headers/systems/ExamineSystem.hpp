#pragma once

#include <vector>
#include <string>
#include "entt.hpp"
#include "Cave.hpp"

namespace ExamineSystem
{
	std::vector<entt::entity> get_clicked_entities(Cave& cave, const size_t player_idx);
	std::vector<std::string> get_info_neat(const entt::registry& registry, const entt::entity entity);
	void show_entities_info(entt::registry& registry, std::vector<entt::entity>& entities);
};
