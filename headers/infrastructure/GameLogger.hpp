#pragma once

#include <cstddef>
#include <sstream>
#include <string>
#include <vector>

#include "external/entt/entt.hpp"

class GameLogger
{
	public:
		class Stream
		{
			public:
				Stream(GameLogger& logger, const entt::registry& registry);

				Stream(const Stream&) = delete;
				Stream& operator=(const Stream&) = delete;

				Stream(Stream&& other) noexcept;
				Stream& operator=(Stream&&) = delete;

				~Stream();

				template<typename T>
					Stream& operator<<(const T& value)
					{
						buffer_ << value;
						return *this;
					}

				Stream& operator<<(entt::entity entity);

				using Manipulator = std::ostream& (*)(std::ostream&);

				Stream& operator<<(Manipulator manipulator)
				{
					manipulator(buffer_);
					return *this;
				}

			private:
				GameLogger* logger_;
				const entt::registry* registry_;
				std::ostringstream buffer_;
		};

		[[nodiscard]]
			Stream stream(const entt::registry& registry);

		[[nodiscard]]
			std::vector<std::string> get_last_messages(std::size_t count) const;

		void log_message(std::string message);

		void clear();

		[[nodiscard]]
			bool empty() const noexcept;

		[[nodiscard]]
			std::size_t size() const noexcept;

	private:
		std::vector<std::string> messages_;
};
