#pragma once

#include <nlohmann/json.hpp>

namespace Editor
{
	void print_json(const nlohmann::json& json, const size_t size = 4);
	void start();
};
