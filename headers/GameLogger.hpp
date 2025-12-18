#pragma once

#include <vector>
#include <string>

class GameLogger
{
	private:
		std::vector<std::string> messages;

	public:
		std::vector<std::string> last(size_t n) const;
		void log(const std::string& message);
};
