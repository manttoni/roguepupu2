#pragma once

#include <algorithm>
#include <string>
#include <vector>

class Messages
{
	private:
		std::vector<std::string> descriptions;

	public:
		void add(const std::vector<std::string>& desc)
		{
			descriptions.insert(descriptions.end(), desc.begin(), desc.end());
		}
		void add(const std::string& desc) { descriptions.push_back(desc); }
		std::vector<std::string> get_last(std::size_t count) const
		{
			count = std::min(count, descriptions.size());
			return {descriptions.end() - count, descriptions.end()};
		}
};
