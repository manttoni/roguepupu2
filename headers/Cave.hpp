#pragma once

#include <string>     // for string
#include "Cell.hpp"   // for Cell
#include "Color.hpp"  // for Color
#include "entt.hpp"   // for size_t, vector, pair
#include "Utils.hpp"

class World;
class Cave
{
	private:
		World* world;
	public:
		World* get_world() { return world; }
		void set_world(World* world) { this->world = world; }

	private:
		size_t height, width;
	public:
		size_t get_height() const { return height; }
		size_t get_width() const { return width; }
		size_t get_size() const { return cells.size(); }

	private:
		std::vector<Cell> cells;
	public:
		const std::vector<Cell>& get_cells() const { return cells; }
		std::vector<Cell>& get_cells() { return cells; }
		Cell& get_cell(const size_t idx) { return cells[idx]; }
		const Cell& get_cell(const size_t idx) const { return cells[idx]; }

	private:
		size_t level;
	public:
		size_t get_level() const { return level; }
		void set_level(const size_t level) { this->level = level; }

	private:
		size_t seed;
	public:
		int get_seed() const { return seed; }
		void set_seed(const int seed) { this->seed = seed; }

	private:
		size_t source, sink;
	public:
		size_t get_source_idx() const { return source; }
		size_t get_sink_idx() const { return sink; }
		Cell& get_source() { return cells[source]; }
		Cell& get_sink() { return cells[sink]; }
		const std::pair<size_t, size_t> get_ends() const { return { source, sink }; }
		void set_source_idx(const size_t source) { this->source = source; }
		void set_sink_idx(const size_t sink) { this->sink = sink; }

	private:
		std::vector<entt::entity> npcs;
	public:
		std::vector<entt::entity>& get_npcs() { return npcs; }

	private:
		double humidity;
	public:
		double get_humidity() const { return humidity; }
		void set_humidity(const double value) { humidity = value; }

	public:
		Cave();
		Cave(const size_t level, const size_t height, const size_t width, const size_t seed);
		Cave(const std::string& map, const size_t width);

		Cave(const Cave& other) = default;
		Cave& operator=(const Cave& other) = default;
		Cave(Cave&& other) = default;
		Cave& operator=(Cave&& other) = default;

		/* CELL TO CELL */
		std::vector<size_t> find_path(const size_t start, const size_t end);
		double distance(const Cell &start, const Cell &end) const;
		double distance(const size_t start_id, const size_t end_id) const;
		std::vector<size_t> get_nearby_ids(const size_t& middle, const double r) const;
		bool has_access(const size_t from_idx, const size_t to_idx) const;
		bool neighbor_has_type(const size_t middle, const Cell::Type type) const;
		bool has_vision(const size_t from, const size_t to, const double vision_range = 0) const;
		Vec2 get_direction(const size_t from, const size_t to);
		void reset_lights();
		void apply_lights();
		void clear_lights();
		void draw();
};
