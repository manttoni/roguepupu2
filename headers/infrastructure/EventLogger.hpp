#pragma once

#include <vector>
#include <string>
#include "domain/Event.hpp"

class EventLogger
{
	private:
		std::vector<Event> events;

	public:
		EventLogger() = default;

		std::vector<Event> get_last_events(size_t n = 1) const;
		std::vector<std::string> get_last_messages(size_t n = 1) const;
		void log_event(Event& event);
};
