#pragma once

#include <string>
#include "Entity.hpp"

class Creature : public Entity
{
	private:
		size_t vision_range;
	public:
		size_t get_vision_range() const { return vision_range; }
		void set_vision_range(const size_t vision_range) { this->vision_range = vision_range; }

	public:
		Creature();
		~Creature();
		Creature(	const std::string& name,
					const wchar_t wchar,
					const Color& color,
					const double vision_range);
		Creature(const Creature& other);
		Creature& operator=(const Creature& other);
};
