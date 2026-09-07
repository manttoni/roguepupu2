#pragma once

#include <cstddef>

#include <panel.h>

#include "utils/Vec2.hpp"
#include "Window.hpp"

namespace Ncurses
{
	class Panel
	{
		private:
			Window window;
			PANEL* ptr = nullptr;

		public:
			Panel(
					int height,
					int width,
					int y,
					int x);

			Panel();
			~Panel();

			Panel(const Panel&) = delete;
			Panel& operator=(const Panel&) = delete;

			Panel(Panel&& other) noexcept;
			Panel& operator=(Panel&& other) noexcept;

			PANEL* get_ptr() noexcept;
			const PANEL* get_ptr() const noexcept;

			Window& get_window() noexcept;
			const Window& get_window() const noexcept;

			void show();
			void hide();

			bool hidden() const;

			void bring_to_top();
			void send_to_bottom();

			void set_position(int y, int x);

			Vec2<int> dimensions() const;
			Vec2<int> position() const;

			bool valid() const { return ptr != nullptr && window.get_ptr() != nullptr; }
	};
}
