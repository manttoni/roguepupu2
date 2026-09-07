#include <nlohmann/json.hpp>
#include <assert.h>
#include <stddef.h>
#include <nlohmann/json_fwd.hpp>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <string>

#include "ncurses/Color.hpp"
#include "generation/CaveGenerator.hpp"
#include "utils/Error.hpp"
#include "utils/Log.hpp"
#include "utils/Parser.hpp"

namespace Parser
{
	using Json = nlohmann::json;

	/*Ncurses::Color parse_color(const Json& data)
	{
		if (data.is_array() && data.size() == 3)
		{
			if (!data[0].is_number() || !data[1].is_number() || !data[2].is_number())
				Error::fatal("Ncurses::Color needs three numeric values");
			const int r = data[0].get<int>();
			const int g = data[1].get<int>();
			const int b = data[2].get<int>();
			if (r < 0 || r > 1000 || g < 0 || g > 1000 || b < 0 || b > 1000)
				Error::fatal("Ncurses::Color value out of bounds [0,1000]");
			return Ncurses::Color(r, g, b);
		}
		if (data.is_object())
		{
			if (!data.contains("red") || !data.contains("green") || !data.contains("blue"))
				Error::fatal("Ncurses::Color needs correct channels");
			if (!data["red"].is_number_integer() || !data["green"].is_number_integer() || !data["blue"].is_number_integer())
				Error::fatal("Ncurses::Color channel value not integer: " + data.dump(4));
			const int r = data["red"].get<int>();
			const int g = data["green"].get<int>();
			const int b = data["blue"].get<int>();
			if (r < 0 || r > 1000 || g < 0 || g > 1000 || b < 0 || b > 1000)
				Error::fatal("Ncurses::Color value out of bounds [0,1000]");
			return Ncurses::Color(r, g, b);
		}
		Error::fatal("Ncurses::Color format is wrong: " + data.dump(4));
	}*/

	Json read_json_file(const std::filesystem::path& path)
	{
		std::ifstream file(path);
		if (!file.is_open())
			Error::fatal("Could not open file: " + path.string());
		Json defs;
		try {
			file >> defs;
		} catch (const Json::parse_error& e) {
			Error::fatal("File: " + path.string() + ", error: " + e.what());
		}
		file.close();
		return defs;
	}

	Json parse_json_array(const std::filesystem::path& path)
	{
		auto data = read_json_file(path);
		assert(data.is_array() || data.empty());
		return data;
	}



};
