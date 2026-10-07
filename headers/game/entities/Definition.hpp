#pragma once

#include <string>

namespace Game::Entity
{
	using Json = nlohmann::json;
	struct Definition
	{
		using ID = std::string;
		ID id;
		Json data;
	};
}
