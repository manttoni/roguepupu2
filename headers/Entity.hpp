#pragma once

#include <string>
#include <memory>
#include "Light.hpp"
#include "Utils.hpp"
#include "Color.hpp"

class Cell;

class Entity
{
	public:
		enum class Type
		{
			NONE,
			FUNGUS,
			CREATURE,
		};
	protected:
		Type type;
	public:
		Type get_type() const { return type; }
		void set_type(const Type type) { this->type = type; }

	protected:
		std::string name;
	public:
		std::string get_name() const {
			return name;
		}
		void set_name(const std::string& name) {
			this->name = name;
		}

	protected:
		wchar_t symbol;
	public:
		wchar_t get_symbol() const { return symbol; }
		void set_symbol(const wchar_t symbol) { this->symbol = symbol; }

	protected:
		Color color;
	public:
		Color get_color() const { return color; }
		void set_color(const Color& color) { this->color = color; }

	/* The host cells pointer */
	protected:
		Cell* cell;
	public:
		auto get_cell() { return cell; }
		void set_cell(Cell* cell) { this->cell = cell; }
		size_t get_idx() const;

	protected:
		std::vector<Light> lights;
	public:
		auto& get_lights() {
			return lights;
		}

	public:
		bool blocks_movement() const;
		bool blocks_vision() const;

	public:
		Entity();
		virtual ~Entity() = 0;
		Entity(	const Type type,
				const std::string& name,
				const wchar_t wchar,
				const Color& color,
				std::vector<Light> lights = {});
		Entity(const Entity& other);
		Entity& operator=(const Entity& other);
		bool operator==(const Entity& other);


		double move(const Direction d);
};
