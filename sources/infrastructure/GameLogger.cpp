#include <string>          // for basic_string, string
#include <vector>          // for vector
#include "infrastructure/GameLogger.hpp"  // for GameLogger

std::vector<std::string> GameLogger::last(size_t n) const
{
	n = std::min(messages.size(), n);
	std::vector<std::string> last_messages(messages.end() - n, messages.end());
	return last_messages;
}

void GameLogger::log(const std::string& message)
{
	messages.push_back(message);
}
