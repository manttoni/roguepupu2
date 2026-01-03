#pragma once

#include "nlohmann/json.hpp"
#include "Event.hpp"

namespace Parser
{
	Effect parse_effect(const nlohmann::json& data);
	Conditions parse_conditions(const nlohmann::json& data);
	Target parse_target(const nlohmann::json& data);
};

