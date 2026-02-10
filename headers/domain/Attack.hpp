#pragma once

#include <vector>
#include <string>
#include "domain/Damage.hpp"
#include "domain/Attribute.hpp"

struct Attack
{
	std::string id = "";
	Damage base_damage;
	double range = 1.5;
	bool is_melee = true;
	std::vector<Attribute> damage_attributes;
	std::vector<Attribute> hit_chance_attributes;
};
