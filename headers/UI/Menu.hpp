#pragma once


class Menu
{
	public:
		struct Element
		{
			enum class Type { None, Text, Button, TextField, ValueSelector, Checkbox };

			// Core
			Type type = Type::None;
			std::string label = "";

			// TextField
			std::optional<size_t> max_input;
			std::optional<std::string> input;

			// ValueSelector
			std::optional<int> min_value, max_value;
			std::optional<int> value;

			// Checkbox
			std::optional<bool> check;

			Element(const Type type, const std::string& label)
				: type(type), label(label) {}

			size_t get_size() const;
			std::string get_text() const;
		};

	public:
		Menu(const Vec2 position);
		~Menu();

	private:
		Vec2 position;
		PANEL* panel;
		std::vector<Element> elements;
		size_t height, width;

		void set_panel();

	public:
		bool add_element(Element& element); // validates elements
		Element get_selection(const size_t default_selected = 0) const;
		std::vector<Element> get_selections(const size_t default_selected = 0) const;
};
