#pragma once

#include <filesystem>
#include <string>
#include <string_view>

#include <nlohmann/json_fwd.hpp>

namespace IO
{
	namespace Paths
	{
		inline const std::filesystem::path entities_file{
			"data/entities.json"
		};
	}

	using Json = nlohmann::json;

	Json read_json(const std::filesystem::path& path);
	bool write_json(const std::filesystem::path& path, const Json& data);

	std::string read_text(const std::filesystem::path& path);
	bool write_text(const std::filesystem::path& path, std::string_view text);
	bool append_text(const std::filesystem::path& path, std::string_view text);
}
