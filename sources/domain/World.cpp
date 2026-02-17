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

size_t World::new_cave(size_t size, const Cell::Type fill)
{
	if (size == SIZE_MAX)
		size = Parser::read_json_file("data/generation/cave/sizes.json")["default"].get<size_t>();
	Cave cave(size, fill);
	const size_t idx = caves.size();
	cave.set_idx(idx);
	caves.push_back(std::move(cave));
	return idx;
}
