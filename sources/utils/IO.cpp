#include "utils/IO.hpp"

#include <fstream>
#include <iterator>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace IO
{
	Json read_json(const std::filesystem::path& path)
	{
		std::ifstream file(path);

		if (!file)
		{
			throw std::runtime_error(
				"Failed to open JSON file: " + path.string()
			);
		}

		return Json::parse(file);
	}

	bool write_json(
		const std::filesystem::path& path,
		const Json& data
	)
	{
		std::ofstream file(path);

		if (!file)
			return false;

		file << data.dump(4) << '\n';
		file.flush();

		return file.good();
	}

	std::string read_text(const std::filesystem::path& path)
	{
		std::ifstream file(path, std::ios::binary);

		if (!file)
		{
			throw std::runtime_error(
				"Failed to open text file: " + path.string()
			);
		}

		return std::string{
			std::istreambuf_iterator<char>{file},
			std::istreambuf_iterator<char>{}
		};
	}

	bool write_text(
		const std::filesystem::path& path,
		const std::string_view text
	)
	{
		std::ofstream file(path, std::ios::binary);

		if (!file)
			return false;

		file.write(
			text.data(),
			static_cast<std::streamsize>(text.size())
		);
		file.flush();

		return file.good();
	}

	bool append_text(
		const std::filesystem::path& path,
		const std::string_view text
	)
	{
		std::ofstream file(
			path,
			std::ios::binary | std::ios::app
		);

		if (!file)
			return false;

		file.write(
			text.data(),
			static_cast<std::streamsize>(text.size())
		);
		file.flush();

		return file.good();
	}
}
