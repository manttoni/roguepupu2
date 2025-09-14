#pragma once
#include <string>
#include "Entity.hpp"

class Fungus : public Entity
{
	public:
		enum class Type
		{
			NONE = 0,
			GLOWING,
		};
		static const std::vector<std::string> names;


	private:
		Type type;

	public:
		Fungus();
		Fungus(const Type type);

};
