#pragma once

#include <stddef.h>  // for size_t
#include <string>    // for basic_string, string
#include <vector>    // for vector

class GameLogger
{
	private:
		std::vector<std::string> messages;

	public:
		std::vector<std::string> last(size_t n) const;
		void log(const std::string& message);
};
