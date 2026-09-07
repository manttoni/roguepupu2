#include "ui/Theme.hpp"
#include "utils/Parser.hpp"
#include <array>

namespace UI
{
	Theme load_theme(const std::filesystem::path& path)
	{
		const auto json = Parser::read_json_file(path);

		return Theme{
			.background{json.at("background").get<std::array<int, 3>>()},
				.text{json.at("text").get<std::array<int, 3>>()},
				.border{json.at("border").get<std::array<int, 3>>()},
				.rock{json.at("rock").get<std::array<int, 3>>()},
				.floor{json.at("floor").get<std::array<int, 3>>()}
		};
	}
}
