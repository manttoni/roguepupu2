#include <string>
#include <map>
#include "Fungus.hpp"
#include "UI.hpp"

const std::map<Fungus::Type, Fungus::Info> Fungus::LUT = {
	{
		Fungus::Type::NONE,
		{
			Cell::Type::NONE,
			"default",
			0,
			'?'
		}
	},
	{
		Fungus::Type::GLOWING,
		{
			Cell::Type::ROCK,
			"Glowing mushroom",
			UI::instance().GLOWING_FUNGUS,
			'*'
		}
	}
};

Fungus::Fungus() :
	Entity("default", 0),
	type(Type::NONE)
{}
Fungus::Fungus(const Type type) :
	Entity(	LUT.at(type).name,
			UI::instance().GLOWING_FUNGUS),
			//LUT.at(type).color_pair_id),
	type(type)
{
	switch (type)
	{
		case Type::GLOWING:
			Entity::set_char(LUT.at(type).ch);
			break;
		default:
			break;
	}
}

