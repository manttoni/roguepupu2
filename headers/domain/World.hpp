#pragma once

#include <vector>
#include "domain/Cave.hpp"
#include "domain/Cell.hpp"

class World
{
	private:
		std::vector<Cave> caves;

	public:
		World() = default;
		Cave& get_cave(const size_t idx);
		const Cave& get_cave(const size_t idx) const;

		size_t new_cave(const size_t size = 150, const Cell::Type fill = Cell::Type::Rock);
};

