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
			short color_pair_id;
			char ch;
			Effect effect;
		};
		Info get_info(const Type type);

	private:
		Type type;

	public:
		Fungus();
		Fungus(const Type type);

};
