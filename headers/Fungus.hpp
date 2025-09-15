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
			COUNT,
		};
		struct Info
		{
			Cell::Type cell_type;
			std::string name;
			short color_pair_id;
			char ch;
		};
		static const std::map<Type, Info> LUT;

	private:
		Type type;

	public:
		Fungus();
		Fungus(const Type type);

};
