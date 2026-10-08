#pragma once

#include <string>
#include <string_view>
#include <iosfwd>

namespace UI
{
	/* When user chooses a button in a menu, that button will be returned wrapped in a Selection.
	 * Sometimes no button was selected, and the details are in state
	 * */
	struct Selection
	{
		enum class State
		{
			Pending, // nothing yet
			Selected, // button was selected and enter was pressed
			Confirmed, // same but it was a confirming element like button "OK"
			Cancelled, // Button like "Cancel" or pressing ESC
			TimedOut, // if menu is not blocking, it can return before anything is selected
			Error,
			SingleChoice,
			MultiChoice,
			Ignored,
			Changed,
		};
		State state = State::Pending;
		size_t index = 0; // index of Element that was selected
		std::string label = "";

		bool pending() const { return state == State::Pending; }
		bool selected() const { return state == State::Selected; }
		bool confirmed() const { return state == State::Confirmed; }
		bool cancelled() const { return state == State::Cancelled; }
		bool timed_out() const { return state == State::TimedOut; }
	};
	std::string_view to_string(Selection::State state);
	std::ostream& operator<<(std::ostream& os, Selection::State state);
	std::ostream& operator<<(std::ostream& os, const Selection& selection);
}
