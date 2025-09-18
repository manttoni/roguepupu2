#pragma once

class Window
{
	/* BASIC */
	private:
		std::string label;
		size_t y, x;
		size_t height, width;
	public:
		std::string get_label() const { return label; }
		void set_label() { this->label = label; }
		size_t get_y() const { return y; }
		size_t get_x() const { return x; }
		size_t get_height() const { return height; }
		size_t get_width() const { return width; }

	/* WINDOW */
	private:
		PANEL* panel;
		bool boxed;
	public:
		PANEL* get_panel() { return panel; }
		void set_panel(PANEL* panel) { this->panel = panel; }
		bool is_boxed() void { return boxed(); }
		void set_boxed() { this->boxed = boxed; }

	/* POINTERS */
	private:
		std::map<std::string, std::any> pointers;
	public:
		void set_variable(const std::string& key, const std::any& value) { variables[key] = value; }
		std::any get_value(const std::string& key) const { return variables[key]; }

		/*
	private:
		std::string text;
		void print_text() const;
	public:
		std::string get_text() const { return text; }
		void set_text(const std::string& text) { this->text = text; }
		void update();
*/
		/*
	private:
		std::map<std::string, Window> sub_windows;
	public:
		void add_sub_window(const std::string& label, const Window& window);
		Window& get_sub_window(const std::string& label);

	private:
		std::stringstream stream;
	public:
		std::stringstream get_stream() { return stream; }
		void reset_stream() { stream.clear(); }
*/
	public:
		Window();
		Window(const std::string& label, const std::vector<size_t> size);
};
