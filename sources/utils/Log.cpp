#include "utils/Log.hpp"
#include "utils/ANSI.hpp"
#include <iomanip>
#include <filesystem>

namespace Log
{
	Stream::Stream(const std::string& label, const ANSI::Code c)
	{
		buffer << c << std::left << std::setw(9) << label << ANSI::RESET << timestamp() << " ";
	}

	Stream::~Stream()
	{
		Log::write(buffer.str());
	}

	Stream stream()
	{
		return Stream{};
	}

	std::string timestamp()
	{
		const auto now = Clock::now();
		std::time_t time = Clock::to_time_t(now);
		std::tm tm = *std::localtime(&time);
		std::ostringstream oss;
		oss << std::put_time(&tm, "[%d.%m.%Y %H:%M:%S]");
		return oss.str();
	}

	void write(const std::string& message, const std::string& filename)
	{
		if (std::filesystem::is_directory("logs"))
			std::filesystem::create_directories("logs");
		std::ofstream os(filename, std::ios::app);
		if (!os)
			throw std::runtime_error("Can't open " + filename);

		os << message << std::endl;
		os.close();
	}

	Stream info()
	{
		return Stream("[INFO]", ANSI::CYAN);
	}
	Stream debug()
	{
		return Stream("[DEBUG]", ANSI::BLUE);
	}
	Stream warning()
	{
		return Stream("[WARNING]", ANSI::YELLOW);
	}
	Stream error()
	{
		return Stream("[ERROR]", ANSI::RED);
	}
};
