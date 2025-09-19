#pragma once

#include <string>
#include "Effect.hpp"

class Cell;
class Entity
{
	private:
		std::string name;
		short color_pair_id;
		char ch;
		Cell* cell;
	public:
		std::string get_name() const {
			return name;
		}
		void set_name(const std::string& name) {
			this->name = name;
		}
		auto get_char() const {
			return ch;
		}
		void set_char(const char ch) {
			this->ch = ch;
		}
		auto get_color_pair_id() const { return color_pair_id; }
		auto get_cell() { return cell; }
		void set_cell(Cell* cell) { this->cell = cell; }
		size_t get_idx() const;

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
		bool blocks_movement() const;
		bool blocks_vision() const;

	public:
		Entity();
		Entity(const std::string& name, const short color_pair_id, const char ch, Cell* cell);



};
