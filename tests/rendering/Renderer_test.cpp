#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <ncurses.h>

#if defined(__SANITIZE_ADDRESS__)
#define ROGUEPUPU_RENDERER_TEST_ASAN
#elif defined(__has_feature)
#if __has_feature(address_sanitizer)
#define ROGUEPUPU_RENDERER_TEST_ASAN
#endif
#endif

#ifdef ROGUEPUPU_RENDERER_TEST_ASAN
#include <sanitizer/lsan_interface.h>
#endif

#include "application/Layout.hpp"
#include "application/Messages.hpp"
#include "game/component/Runtime.hpp"
#include "rendering/Renderer.hpp"

namespace
{
	class RendererTest : public ::testing::Test
	{
		protected:
			FILE* input = nullptr;
			FILE* output = nullptr;
			SCREEN* screen = nullptr;
			std::unique_ptr<Layout> layout;
			Messages messages;
			std::unique_ptr<Renderer> renderer;
#if !NCURSES_REENTRANT
			int previous_lines = LINES;
			int previous_columns = COLS;
#endif

			void SetUp() override
			{
				input = std::tmpfile();
				output = std::tmpfile();
				ASSERT_NE(input, nullptr);
				ASSERT_NE(output, nullptr);
				screen = newterm("xterm-256color", output, input);
				ASSERT_NE(screen, nullptr);
				ASSERT_NE(start_color(), ERR);
				ASSERT_NE(resizeterm(40, 80), ERR);

				// ncurses retains an internal stdscr pseudo-panel after delscreen().
				// Exclude only its initialization; all application panels stay tracked.
				PANEL* initialization_panel = nullptr;
				{
#ifdef ROGUEPUPU_RENDERER_TEST_ASAN
					const __lsan::ScopedDisabler ignore_ncurses_root;
#endif
					initialization_panel = new_panel(stdscr);
				}
				ASSERT_NE(initialization_panel, nullptr);
				ASSERT_EQ(del_panel(initialization_panel), OK);

				layout = std::make_unique<Layout>(Layout::Settings{
					.cross_position = {8, 40}});
				renderer = std::make_unique<Renderer>(*layout, messages);
			}

			void TearDown() override
			{
				renderer.reset();
				layout.reset();
				if (screen != nullptr)
				{
					// The ignored root must not keep application panels reachable.
					EXPECT_EQ(panel_above(nullptr), nullptr);
					endwin();
					delscreen(screen);
				}
				if (output != nullptr)
					std::fclose(output);
				if (input != nullptr)
					std::fclose(input);
#if !NCURSES_REENTRANT
				LINES = previous_lines;
				COLS = previous_columns;
#endif
			}

			Ncurses::Window& window(Layout::Panel panel)
			{
				return layout->get_panel(panel).get_window();
			}

			std::string text_at(Ncurses::Window& surface, int y, int x, int count)
			{
				std::vector<char> text(static_cast<std::size_t>(count) + 1);
				mvwinnstr(surface.get_ptr(), y, x, text.data(), count);
				return text.data();
			}
	};

	TEST_F(RendererTest, RedrawsAllPanelsAndPresentsMessagesWithoutAGame)
	{
		messages.add("older");
		messages.add("newer");
		window(Layout::Panel::Game).write(1, 1, "stale");
		window(Layout::Panel::PlayerStatus).write(1, 1, "stale");

		renderer->redraw();

		auto& log = window(Layout::Panel::MessageLog);
		const auto log_size = log.dimensions();
		EXPECT_EQ(text_at(log, log_size.y - 2, 0, 5), "older");
		EXPECT_EQ(text_at(log, log_size.y - 1, 0, 5), "newer");
		EXPECT_EQ(mvwinch(window(Layout::Panel::Game).get_ptr(), 1, 1) & A_CHARTEXT, ' ');
		EXPECT_EQ(text_at(window(Layout::Panel::PlayerStatus), 1, 1, 5), "     ");
		EXPECT_EQ(mvwinch(log.get_ptr(), 0, 1), ACS_HLINE);
		EXPECT_EQ(mvwinch(window(Layout::Panel::PlayerStatus).get_ptr(), 8, 0), ACS_RTEE);
		EXPECT_EQ(mvwinch(curscr, 39, 0) & A_CHARTEXT, 'n');
	}

	TEST_F(RendererTest, UsesTheFullLogHeightAndReadsNewMessagesOnEveryRedraw)
	{
		for (int i = 0; i < 40; ++i)
			messages.add("message " + std::to_string(i));

		renderer->redraw();

		auto& log = window(Layout::Panel::MessageLog);
		EXPECT_EQ(text_at(log, 1, 0, 9), "message 9");
		EXPECT_EQ(text_at(log, 31, 0, 10), "message 39");

		messages.add("latest");
		renderer->redraw();
		EXPECT_EQ(text_at(log, 1, 0, 10), "message 10");
		EXPECT_EQ(text_at(log, 31, 0, 10), "latest    ");
	}

	TEST_F(RendererTest, MovesBordersWithTheLayoutAndKeepsMenusAboveTheGame)
	{
		Ncurses::Panel menu(3, 14, 2, 2);
		menu.get_window().write(1, 1, "Settings");
		renderer->redraw();

		layout->get_settings().cross_position = {12, 52};
		layout->reset();
		renderer->redraw();

		auto& status = window(Layout::Panel::PlayerStatus);
		EXPECT_EQ(layout->get_panel(Layout::Panel::MessageLog).position().y, 12);
		EXPECT_EQ(layout->get_panel(Layout::Panel::PlayerStatus).position().x, 52);
		EXPECT_EQ(mvwinch(status.get_ptr(), 8, 0), ACS_VLINE);
		EXPECT_EQ(mvwinch(status.get_ptr(), 12, 0), ACS_RTEE);
		EXPECT_EQ(mvwinch(curscr, 12, 52), ACS_RTEE);
		EXPECT_EQ(mvwinch(curscr, 3, 3) & A_CHARTEXT, 'S');
	}

	TEST_F(RendererTest, HandlesAuxiliaryPanelsBeingHiddenAndShownAfterResize)
	{
		messages.add("retained");
		ASSERT_NE(resizeterm(2, 2), ERR);
		layout->reset();
		ASSERT_TRUE(layout->get_panel(Layout::Panel::MessageLog).hidden());
		ASSERT_TRUE(layout->get_panel(Layout::Panel::PlayerStatus).hidden());
		EXPECT_NO_THROW(renderer->redraw());

		ASSERT_NE(resizeterm(40, 80), ERR);
		layout->reset();
		renderer->redraw();

		EXPECT_FALSE(layout->get_panel(Layout::Panel::MessageLog).hidden());
		EXPECT_FALSE(layout->get_panel(Layout::Panel::PlayerStatus).hidden());
		EXPECT_EQ(text_at(window(Layout::Panel::MessageLog), 31, 0, 8), "retained");
		EXPECT_EQ(mvwinch(window(Layout::Panel::PlayerStatus).get_ptr(), 8, 0), ACS_RTEE);
	}

	TEST_F(RendererTest, KeepsTheTargetingCursorAndLogInTheSameRedraw)
	{
		Game::Simulation simulation{
			"item-interactions-test", EntityDatabase{}, "player_default"};
		auto& registry = simulation.get_registry();
		const auto player = simulation.get_player();
		const auto position = registry.get<Game::Component::Value::Position>(player).value;
		auto& vision = registry.get<Game::Component::Runtime::Vision>(player);
		vision.visible_positions = {position};
		vision.seen_positions.clear();
		messages.add("targeting");

		renderer->redraw(&simulation, position);

		auto& game = window(Layout::Panel::Game);
		const auto center = game.content_dimensions() / 2;
		EXPECT_NE(mvwinch(game.get_ptr(), center.y, center.x) & A_REVERSE, 0u);
		EXPECT_EQ(text_at(window(Layout::Panel::MessageLog), 31, 0, 9), "targeting");
		EXPECT_EQ(mvwinch(window(Layout::Panel::PlayerStatus).get_ptr(), 8, 0), ACS_RTEE);

		renderer->redraw(&simulation);
		EXPECT_EQ(mvwinch(game.get_ptr(), center.y, center.x) & A_REVERSE, 0u);
	}
}
