#pragma once

#include <string>
#include "Entity.hpp"

class Creature : public Entity
{
	private:
		size_t vision_range;
		std::vector<size_t> seen_cells;
	public:
		size_t get_vision_range() const { return vision_range; }
		void set_vision_range(const size_t vision_range) { this->vision_range = vision_range; }
		bool can_see_cell(const size_t cell_idx);
		bool has_seen_cell(const size_t cell_idx);

	public:
		Creature();
		~Creature() {}
		Creature(const std::string& name, const short color_id, const char ch = '\0', Cell* cell = nullptr);
};
