#pragma once

namespace Ncurses
{
	class Session
	{
		public:
			Session();
			~Session() noexcept;

			Session(const Session&) = delete;
			Session& operator=(const Session&) = delete;

			Session(Session&&) = delete;
			Session& operator=(Session&&) = delete;
	};
}
