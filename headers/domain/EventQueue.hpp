#pragma once

#include <vector>
#include "Event.hpp"

/* A queue where Events are created when Effect has happened.
 * Event is a description of what Effect happened to who by who.
 * Some Events will trigger other Effects, which is why this
 * queue exists. This queue is also for logging events.
 * */

struct EventQueue
{
	std::vector<Event> queue;
};
