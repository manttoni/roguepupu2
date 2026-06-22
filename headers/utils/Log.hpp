#pragma once

#include <fstream>
#include <string>
#include <chrono>
#include <sstream>
#include "utils/ANSI.hpp"

namespace Log
{
	constexpr const char* logfile = "logs/logs.log";

	using Clock = std::chrono::system_clock;

	class Stream
	{
		private:
			std::ostringstream buffer;

		public:
			Stream(const std::string& label = "", const ANSI::Code c = "");
			Stream(Stream&&) = default;
			Stream& operator=(Stream&&) = default;

			Stream(const Stream&) = delete;
			Stream& operator=(const Stream&) = delete;

			~Stream();

			template <typename T>
				Stream& operator<<(const T& value)
				{
					buffer << value;
					return *this;
				}
			using Manip = std::ostream& (*)(std::ostream&);

			Stream& operator<<(Manip m)
			{
				m(buffer);
				return *this;
			}
	};

	std::string timestamp();
	void write(const std::string& message, const std::string& filename = logfile);

	Stream info();
	Stream debug();
	Stream warning();
	Stream error();
}

