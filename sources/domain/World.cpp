#include <stddef.h>
#include <stdint.h>
#include <nlohmann/json.hpp>
#include <utility>
#include <algorithm>
#include <filesystem>
#include <map>
#include <vector>

#include "domain/World.hpp"
#include "domain/Cave.hpp"
#include "utils/Parser.hpp"
#include "utils/Error.hpp"
#include "domain/Cell.hpp"

nlohmann::json World::load_conf()
{
	return Parser::read_json_file("data/generation/world.json");
}

World::World()
	: max_caves      (load_conf()["max_caves"].get<size_t>()),
	max_depth      (load_conf()["max_depth"].get<size_t>()),
	max_connections(load_conf()["max_connections"].get<size_t>()),
	cave_size      (load_conf()["cave_size"].get<size_t>())
{
	assert(max_caves > max_depth);
	assert(max_connections > 0);

	// Create main branch
	for (size_t i = 0; i < max_depth; ++i)
	{
		caves.emplace_back(i, cave_size, i);
		auto& cave = caves[i];
		if (i < max_depth - 1)
			cave.add_connection(i + 1);
		if (i > 0)
			cave.add_connection(i - 1);
	}
}

Cave& World::get_cave(const size_t idx)
{
	if (idx >= caves.size())
		Error::fatal("Cave does not exist");
	return caves[idx];
}

const Cave& World::get_cave(const size_t idx) const
{
	if (idx >= caves.size())
		Error::fatal("Cave does not exist");
	return caves[idx];
}

size_t World::new_cave(const size_t size, const Cell::Type fill)
{
	const size_t idx = caves.size();
	caves.emplace_back(idx, size, 0, fill);
	return idx;
}
