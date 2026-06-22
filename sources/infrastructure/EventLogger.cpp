#include "infrastructure/EventLogger.hpp"
#include "utils/Log.hpp"

std::vector<Event> EventLogger::get_last_events(size_t n) const
{
	n = std::min(events.size(), n);
	std::vector<Event> last_events(events.end() - n, events.end());
	return last_events;
}

std::vector<std::string> EventLogger::get_last_messages(size_t n) const
{
	n = std::min(events.size(), n);
	std::vector<std::string> last_messages;
	const auto last_events = get_last_events(n);
	for (auto e : last_events)
		last_messages.push_back(e.message);
	return last_messages;
}

void EventLogger::log_event(Event& event)
{
	events.push_back(event);
}
