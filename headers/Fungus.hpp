#pragma once
#include <string>
#include "Entity.hpp"
#include "Cell.hpp"
#include "Effect.hpp"

class Fungus : public Entity
{
	private:

	public:
		Fungus(	const std::string& name,
				const wchar_t wchar,
				const Color& color,
				const std::vector<Light>& lights = {});
		Fungus();
		~Fungus();
		Fungus(const Fungus& other);
		Fungus& operator=(const Fungus& other);

};
