#pragma once

#include <string>
#include "Entity.hpp"

class Creature : public Entity
{
	private:

	public:
		Creature();
		~Creature() {}
		Creature(const std::string& name, const short color_id, const char ch = '\0', Cell* cell = nullptr);
};
