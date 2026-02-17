#pragma once

#include <stdexcept>
#include "utils/Log.hpp"

namespace Error
{
	[[noreturn]] inline void fatal(const std::string& message)
	{
		Log::log(message);
		throw std::runtime_error("Fatal error: " + message);
	}

	inline void warning(const std::string& message)
	{
		Log::log("Warning: " + message);
	}
};
