#pragma once

#include <vector>
#include "domain/Cave.hpp"
#include "domain/Cell.hpp"

class World
{
	private:
		std::vector<Cave> caves;
		const size_t max_caves;
		const size_t max_depth;
		const size_t max_connections;
		const size_t cave_size;
		static nlohmann::json load_conf();

	public:
		World();
		Cave& get_cave(const size_t idx);
		const Cave& get_cave(const size_t idx) const;

		size_t new_cave(const size_t size, const Cell::Type fill);
};

