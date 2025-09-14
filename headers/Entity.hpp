#pragma once

#include <string>
#include "Utils.hpp"
#include "Effect.hpp"

class Entity
{
	private:
		std::string name;
		char ch;
	public:
		std::string get_name() const {
			return name;
		}
		void set_name(const std::string& name) {
			if (this->name.front() == ch)
				ch = name.front();
			this->name = name;
		}
		auto get_char() const {
			return ch;
		}
		void set_char(const char ch) {
			this->ch = ch;
		}

	private:
		std::vector<Effect> effects; // f.e. blue glow from a fungus
	public:
		auto get_effects() const {
			return effects;
		}
		void set_effects(const std::vector<Effect>& effects) {
			this->effects = effects;
		}
		void add_effect(const Effect& effect) {
			effects.push_back(effect);
		}


	public:
		Entity();
		Entity(const std::string& name);


};
