#pragma once
#include <string>
#include "Entity.hpp"
#include "Cell.hpp"

class Fungus : public Entity
{
	public:
		enum class Type
		{
			NONE = 0,
			GLOWING,
			WOODY,
			COUNT,
		};
		struct Info
		{
			std::string name;
			Color color;
			char ch;
			Light light;
		};
		Info get_info(const Type type);

	private:
		Type type;

	public:
		Fungus();
		Fungus(const Type type, Cell* cell);

};
