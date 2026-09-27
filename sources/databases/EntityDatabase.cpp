#include "databases/EntityDatabase.hpp"
#include "utils/Log.hpp"
#include "utils/IO.hpp"
#include "utils/Parser.hpp"
#include "game/entities/Entity.hpp"

#include <stdexcept>
#include <iostream>

EntityDatabase::EntityDatabase(const std::filesystem::path& path)
{
	if (path.extension() != ".json")
		throw std::invalid_argument(
				"Entity database must be a JSON file: " +
				path.string());

	const auto root = IO::read_json(path);

	assert(!root.empty());

	if (!root.is_object())
		throw std::runtime_error("Entity database root must be an object");

	definitions.reserve(root.size());

	for (const auto& [id, definition] : root.items())
	{
		if (!Game::Entity::valid_id(id))
		{
			Log::warning() << "Invalid entity id: " << id << " ignored";
			continue;
		}
		if (!Game::Entity::valid_definition(definition))
		{
			Log::warning() << "Invalid entity(" << id << ") definition:\n" << definition.dump(4);
			continue;
		}

		definitions.emplace(id, definition);
	}

	Log::debug() << path << " entities parsed. Found " << definitions.size() << " entities";
}
