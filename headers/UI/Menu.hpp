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
			std::optional<size_t> max_input;
			std::optional<std::string> input;

			// ValueSelector
			std::optional<int> min_value, max_value;
			std::optional<int> value;

			// Checkbox
			std::optional<bool> check;

			Element() = default;
			Element(const Type type, const std::string& label)
				: type(type), label(label) {}

			size_t get_size() const;
			std::string get_text() const;

			/*operator size_t() const { return index; }
			operator std::string() const { return label; }
			friend bool operator==(const Element& e, const std::string& s)
			{
				return e.label == s;
			}
			friend bool operator==(const std::string& s, const Element& e)
			{
				return e.label == s;
			}*/
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
		bool selection_confirmed(const Element& e, const int key) const;

	public:
		bool add_element(Element element); // validates elements
		Element get_selection(const size_t default_selected = 0);
		std::vector<Element> get_selections(const size_t default_selected = 0);
};
