#pragma once

#include <string>
#include <optional>
#include <panel.h>
#include <vector>
#include <variant>
#include "utils/Vec2.hpp"
#include "utils/Screen.hpp"

class Menu
{
	public:
		struct Element
		{
			using ElementValue = std::variant<bool*, int*, std::string*, double*>;
			enum class Type { None, Text, Button, TextField, ValueSelector, Checkbox };

			Type type = Type::None;
			std::string label = "";
			size_t index = 0; // set when returned in 'get_selection'
			ElementValue value;
			int min_value, max_value; // length of string, limits of numbers

			Element() = default;
			Element(const Type type, const std::string& label, const ElementValue value = ElementValue{(bool*)nullptr}, const int min_value = 0, const int max_value = 10)
				: type(type), label(label), value(value), min_value(min_value), max_value(max_value)
			{}

			size_t get_size() const;
			std::string get_text() const;
		};

	public:
		Menu(const Vec2& position = Screen::middle());
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
};
