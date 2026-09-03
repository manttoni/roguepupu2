#include "database/EntityDatabase.hpp"
#include "utils/Parser.hpp"
#include "entities/Entity.hpp"

#include <stdexcept>

EntityDatabase::EntityDatabase(const std::filesystem::path& path)
{
	if (path.extension() != ".json")
		throw std::invalid_argument(
				"Entity database must be a JSON file: " +
				path.string());

	const auto root = Parser::read_json_file(path);

	if (!root.is_object())
		throw std::runtime_error("Entity database root must be an object");

	definitions.reserve(root.size());

	for (const auto& [id, definition] : root.items())
	{
		if (!Entity::valid_id(id))
		{
			Log::warning() << "Invalid entity id: " << id << " ignored";
			continue;
		}
		if (!Entity::valid_definition(definition))
		{
			Log::warning() << "Invalid entity(" << id << ") definition: " << std::endl << definition.dump(4);
			continue;
		}

		definitions.emplace(id, definition);
	}
}
