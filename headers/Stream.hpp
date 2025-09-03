#pragma once

#include <cstddef>

class Stream
{
	private:
		size_t source_id, sink_id;

	public:
		Stream(const size_t source_id, const size_t sink_id);
};
