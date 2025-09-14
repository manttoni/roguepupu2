#include <string>
#include <map>
#include "Fungus.hpp"

const std::map<Fungus::Type, Fungus::Info> Fungus::LUT = {
	{ Fungus::Type::NONE,   { Cell::Type::NONE, "default" } },
	{ Fungus::Type::GLOWING, { Cell::Type::ROCK, "Glowing mushroom" } }
};

Fungus::Fungus() :
	Entity("default"),
	fungus_type(Type::NONE)
{}
Fungus::Fungus(const Type fungus_type) :
	Entity(get_name(fungus_type)),
	fungus_type(fungus_type)
{}
