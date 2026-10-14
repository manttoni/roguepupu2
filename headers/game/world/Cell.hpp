#pragma once

#include <string>
#include "game/Enum.hpp"

namespace Game::World
{
	struct Cell
	{
		Enum::Material material = Enum::Material::None;
		Enum::Form form = Enum::Form::None;
		double water_depth{};

		inline static std::string to_string(const Cell& cell)
		{
			std::string string;
			string += "{ ";
			string += "material: " + Enum::to_string<Enum::Material>(cell.material);
			string += ", form: " + Enum::to_string<Enum::Form>(cell.form);
			string += " }";
			return string;
		}
	};
}
