#pragma once
#include <thread>
#include <chrono>
namespace Time
{
	void sleep(const size_t ms)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	}
};
