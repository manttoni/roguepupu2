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
			std::string_view name;
		};
		static const std::map<Type, Info> LUT;
		Cell::Type get_affinity(Type type) const { return LUT.at(type).cell_type; }
		Cell::Type get_affinity() const { return get_affinity(fungus_type); }
		std::string get_name(Type type) const { return std::string(LUT.at(type).name); }
		std::string get_name() const { return get_name(fungus_type); }

	private:
		Type fungus_type;

	public:
		Fungus();
		Fungus(const Type type);

};
