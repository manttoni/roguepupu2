#pragma once

#include <stddef.h>

namespace Tester
{
	struct Results
	{
		size_t failed_tests = 0;
		bool log_success = false;
	};
	extern Results test_results;

	bool test();
};
