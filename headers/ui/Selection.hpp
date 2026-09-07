#pragma once

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

		inline bool cancelled() const { return state == State::Cancelled; }
		inline bool confirmed() const { return state == State::Confirmed; }
		inline bool timed_out() const { return state == State::TimedOut; }
		inline bool selected() const { return state == State::Selected; }
		inline bool pending() const { return state == State::Pending; }
	};
}
