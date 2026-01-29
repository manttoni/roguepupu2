#pragma once

#include <string>
#include <optional>
#include <panel.h>
#include <vector>
#include "utils/Vec2.hpp"

class Menu
{
	public:
		struct Element
		{
			enum class Type { None, Text, Button, TextField, ValueSelector, Checkbox };

			// Core
			Type type = Type::None;
			std::string label = "";
			size_t index = SIZE_MAX; // index in elements vector. Needed by f.e. ContextSystem

			// TextField
			std::optional<size_t> min_input, max_input;
			std::optional<std::string> input;

			// ValueSelector
			std::optional<int> min_value, max_value;
			std::optional<int> value;

			// Checkbox
			std::optional<bool> check;

			Element() = default;

			// For any type
			Element(const Type type, const std::string& label)
				: type(type), label(label) {}

			// For TextField
			Element(const Type type, const std::string& label, const size_t min_input, const size_t max_input)
				:type(type), label(label), min_input(min_input), max_input(max_input) { assert(type == Type::TextField); }

			// For ValueSelector
			Element(const Type type, const std::string& label, const int min_value, const int max_value)
				: type(type), label(label), min_value(min_value), max_value(max_value) { assert(type == Type::ValueSelector); }

			// For Checkbox
			Element(const Type type, const std::string& label, const bool check)
				: type(type), label(label), check(check) { assert(type == Type::Checkbox); }

			size_t get_size() const;
			std::string get_text() const;
		};

	public:
		Menu(const Vec2& position);
		~Menu();

	private:
		Vec2 position;
		PANEL* panel;
		std::vector<Element> elements;
		size_t height, width;

		void set_panel();
		size_t get_unselectable_count() const;
		int get_mouse_selection() const;
		void show_elements(const size_t selected) const;
		size_t select_element(const size_t selected, const int key) const;
		void change_value(Element& e, const int key);
		void input_text(Element& e, const int key);
		void set_bool(Element& e, const int key);
		bool selection_confirmed(const Element& e, const int key) const;

	public:
		bool add_element(Element element); // validates elements
		Element get_selection(const size_t default_selected = 0);
		int get_value(const std::string& label) const;
		std::string get_input(const std::string& label) const;
		bool get_check(const std::string& label) const;
};
