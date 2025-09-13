#pragma once

#include <string>

class Entity
{
	private:
		std::string name;

	public:
		Entity();
		Entity(const std::string& name);
		std::string get_name() const { return name; }
		char get_char() const {return name.front(); }
};
