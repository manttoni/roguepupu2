#pragma once

#include <utility>
#include "nlohmann/json.hpp"
#include "Event.hpp"

namespace Parser
{
	Effect parse_effect(const nlohmann::json& data);
	Conditions parse_conditions(const nlohmann::json& data);
	Target parse_target(const nlohmann::json& data);
	std::pair<size_t, size_t> parse_range(const nlohmann::json& data);
};

