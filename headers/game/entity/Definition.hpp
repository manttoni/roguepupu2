#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace Game::Entity
{
	using Json = nlohmann::json;
	struct Definition
	{
		using ID = std::string;
		ID id;
		Json data;

		bool operator==(const Definition& other) const = default;
	};
}
