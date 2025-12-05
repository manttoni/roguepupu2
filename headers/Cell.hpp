#pragma once

#include <memory>
#include <string>
#include <vector>
#include <map>
#include "Utils.hpp"
#include "Color.hpp"
#include "ColorPair.hpp"
#include "entt.hpp"

class Cave;
class Cell
{
	private:
		size_t idx;
	public:
		size_t get_idx() const {
			return idx;
		}
		void set_idx(const size_t i) {
			this->idx = i;
		}

	public:
		enum class Type
		{
			ROCK,	// solid rock
			FLOOR,	// empty space with a floor
			SOURCE,	// previous level
			SINK,	// next level
			NONE,	// default
		};
	private:
		Type type;
	public:
		Type get_type() const {
			return type;
		}
		void set_type(const Cell::Type type) {
			this->type = type;
		}

	private:
		Cave* cave;
	public:
		Cave* get_cave() {
			return cave;
		}
		void set_cave(Cave* cave) {
			this->cave = cave;
		}

	private:
		wchar_t glyph;
	public:
		wchar_t get_glyph() const;
		void set_glyph(const wchar_t glyph) { this->glyph = glyph; }

	private:
		double density;
	public:
		void set_density(const double d) {
			this->density = d;
		}
		double get_density() const {
			return density;
		}
		void reduce_density(const double amount);

	private:
		Color fg, bg;
	public:
		Color get_fg() const { return fg; }
		Color get_bg() const { return bg; }
		void set_fg(const Color& fg) { this->fg = fg; }
		void set_bg(const Color& bg) { this->bg = bg; }
		ColorPair get_color_pair() const;

	private:
		std::map<Color, size_t> lights;
	public:
		auto get_lights() const {
			return lights;
		}
		void add_light(const Color& color) {
			lights[color]++;
		}
		void reset_lights() {
			lights.clear();
		}

	private:
		bool seen = false;
	public:
		bool is_seen() const { return seen; }
		void set_seen(const bool seen) { this->seen = seen; }

	public:
		Cell();
		Cell(const size_t idx, const Type& type, Cave* cave, const wchar_t symbol, const double density = 0);
		Cell(const Cell& other) = default;

		char get_char() const;

		/* OVERLOADS */
		bool operator==(const Cell &other) const;
		bool operator!=(const Cell &other) const;
		bool operator<(const Cell &other) const;
		Cell &operator=(const Cell& other) = default;

		/* CELL FEATURES */
		bool blocks_vision() const;
		bool blocks_movement() const;

		// Get entities in this
		std::vector<entt::entity> get_entities() const;
};
