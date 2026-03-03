#pragma once

#include <nlohmann/json.hpp>

namespace JsonUtils
{
	using Json = nlohmann::json;
	bool contains_all(const Json& super, const Json& sub);
	bool contains_any(const Json& super, const Json& sub);
	bool contains_none(const Json& super, const Json& sub);
	std::string to_string(const Json& j);
	Json merge(const Json& a, const Json& b, const bool replace = false);
};
