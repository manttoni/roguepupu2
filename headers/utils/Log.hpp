#pragma once

#include <sstream>
#include <string>
#include <utility>

namespace Log
{
	class Stream
	{
		public:
			explicit Stream(const char* level);
			~Stream() noexcept;

			Stream(const Stream&) = delete;
			Stream& operator=(const Stream&) = delete;

			template<typename T>
				Stream& operator<<(T&& value)
				{
					buffer << std::forward<T>(value);
					return *this;
				}

		private:
			const char* level;
			std::ostringstream buffer;
	};

	Stream info();
	Stream debug();
	Stream warning();
	Stream error();
}
