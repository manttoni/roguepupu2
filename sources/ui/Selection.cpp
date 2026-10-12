#include "ui/Selection.hpp"
#include <ostream>

namespace UI
{
	std::string_view to_string(Selection::State state)
	{
		using State = Selection::State;

		switch (state)
		{
			case State::Pending:      return "Pending";
			case State::Selected:     return "Selected";
			case State::Confirmed:    return "Confirmed";
			case State::Cancelled:    return "Cancelled";
			case State::TimedOut:     return "TimedOut";
			case State::Error:        return "Error";
			case State::SingleChoice: return "SingleChoice";
			case State::MultiChoice:  return "MultiChoice";
			case State::Ignored:      return "Ignored";
			case State::Changed:      return "Changed";
		}
		return "Unknown";
	}

	std::ostream& operator<<(std::ostream& os, Selection::State state)
	{
		return os << to_string(state);
	}

	std::ostream& operator<<(std::ostream& os, const Selection& selection)
	{
		return os << "Selection{state=" << selection.state
			<< ", index=" << selection.index
			<< ", label=\"" << selection.label << "\"}";
	}
}
