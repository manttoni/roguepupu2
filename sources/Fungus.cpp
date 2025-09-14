#include <string>
#include "Fungus.hpp"

const std::vector<std::string> Fungus::names =
{
	"default",
    "Glowing mushroom"
};
Fungus::Fungus() :
	Entity(names[static_cast<int>(Type::NONE)]),
	type(Type::NONE)
{}
Fungus::Fungus(const Type type) :
	Entity(names[static_cast<int>(type)]),
	type(type)
{}
