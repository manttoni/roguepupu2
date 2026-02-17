#pragma once

#include <string>
#include <optional>
#include <panel.h>
#include <vector>
#include <variant>
#include <nlohmann/json.hpp>
#include "utils/Vec2.hpp"
#include "utils/Screen.hpp"
#include "utils/Range.hpp"

class Menu
{
	public:
		struct Element
		{
			using ElementValue = std::variant<bool*, int*, std::string*, double*, nlohmann::json*>;
			enum class Type { None, Text, Button, TextField, ValueSelector, Checkbox, MultiChoice };

			Type type = Type::None;
			std::string label = "";
			ElementValue value;
			Range<int> value_range;
			std::vector<std::string> choices;
			double delta;
			char highlight = '\0'; // Mark an element with some char
			bool is_cancel = false;
			bool is_confirm = false;

			Element() = default;
			Element(const Type type, const std::string& label) : type(type), label(label) {}
			Element(const Type type,
					const std::string& label,
					const ElementValue& value,// = ElementValue{(bool*)nullptr},
					const Range<int>& range = Range(0, 10),
					const double delta = 1.0)
				: type(type), label(label), value(value), value_range(range), delta(delta)
			{}
			Element(const Type type,
					const std::string& label,
					const ElementValue value,
					const std::vector<std::string>& choices)
				: type(type), label(label), value(value), choices(choices)
			{}

			size_t get_size() const;
			std::string get_text(const size_t width) const; // give menu width for formatting

			static inline Element cancel(const std::string& label = "Cancel") { Element e(Type::Button, label); e.is_cancel = true; return e; }
			static inline Element confirm(const std::string& label = "Confirm") { Element e(Type::Button, label); e.is_confirm = true; return e; }
			static inline Element line() { Element e(Type::Text, "--"); return e; }
			static inline Element text(const std::string& label) { return Element(Type::Text, label); }
		};
		struct Selection
		{
			size_t index = 0;
			std::optional<Element> element;
			bool cancelled = false; // ESC or Cancel/Back/Quit etc button
			bool confirmed = false; // OK or something pressed
			bool timed_out = false; // If menu is non-blocking

			Selection() = default;
			Selection(const size_t index) : index(index) {}
			Selection(const size_t index, const Element& element)
				: index(index), element(element)
			{}

			static inline Selection cancel() { Selection s; s.cancelled = true; return s; }
			static inline Selection confirm() { Selection s; s.confirmed = true; return s; }
			static inline Selection time_out() { Selection s; s.timed_out = true; return s; }
		};

	public:
		Menu(const Vec2& position = Screen::middle());
		~Menu();

	private:
		Vec2 position;
		PANEL* panel;
		std::vector<Element> elements;
		size_t height, width;
		bool blocking = true;

		void set_panel();
		size_t get_unselectable_count() const;
		int get_mouse_selection() const;
		void show_elements(const size_t selected) const;
		size_t select_element(const size_t selected, const int key) const;
		void change_value(Element& e, const int key);
		void input_text(Element& e, const int key);
		void set_bool(Element& e, const int key);
		void select_multi_choice(Element& e, const int key);
		bool selection_confirmed(const Element& e, const int key) const;

	public:
		void add_element(Element element);
		Selection get_selection(const size_t default_selected = 0);
		void set_blocking(const bool value) { blocking = value; }
};
