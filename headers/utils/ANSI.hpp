#pragma once

namespace ANSI
{
	using Code = const char*;

	// Reset
	constexpr Code RESET = "\033[0m";

	// Text styles
	constexpr Code BOLD      = "\033[1m";
	constexpr Code DIM       = "\033[2m";
	constexpr Code ITALIC    = "\033[3m";
	constexpr Code UNDERLINE = "\033[4m";
	constexpr Code BLINK     = "\033[5m";
	constexpr Code REVERSE   = "\033[7m";
	constexpr Code HIDDEN    = "\033[8m";
	constexpr Code STRIKE    = "\033[9m";

	// Standard foreground colors
	constexpr Code BLACK   = "\033[30m";
	constexpr Code RED     = "\033[31m";
	constexpr Code GREEN   = "\033[32m";
	constexpr Code YELLOW  = "\033[33m";
	constexpr Code BLUE    = "\033[34m";
	constexpr Code MAGENTA = "\033[35m";
	constexpr Code CYAN    = "\033[36m";
	constexpr Code WHITE   = "\033[37m";

	// Bright foreground colors
	constexpr Code BRIGHT_BLACK   = "\033[90m";
	constexpr Code BRIGHT_RED     = "\033[91m";
	constexpr Code BRIGHT_GREEN   = "\033[92m";
	constexpr Code BRIGHT_YELLOW  = "\033[93m";
	constexpr Code BRIGHT_BLUE    = "\033[94m";
	constexpr Code BRIGHT_MAGENTA = "\033[95m";
	constexpr Code BRIGHT_CYAN    = "\033[96m";
	constexpr Code BRIGHT_WHITE   = "\033[97m";

	// Background colors
	constexpr Code BG_BLACK   = "\033[40m";
	constexpr Code BG_RED     = "\033[41m";
	constexpr Code BG_GREEN   = "\033[42m";
	constexpr Code BG_YELLOW  = "\033[43m";
	constexpr Code BG_BLUE    = "\033[44m";
	constexpr Code BG_MAGENTA = "\033[45m";
	constexpr Code BG_CYAN    = "\033[46m";
	constexpr Code BG_WHITE   = "\033[47m";

	// Bright background colors
	constexpr Code BG_BRIGHT_BLACK   = "\033[100m";
	constexpr Code BG_BRIGHT_RED     = "\033[101m";
	constexpr Code BG_BRIGHT_GREEN   = "\033[102m";
	constexpr Code BG_BRIGHT_YELLOW  = "\033[103m";
	constexpr Code BG_BRIGHT_BLUE    = "\033[104m";
	constexpr Code BG_BRIGHT_MAGENTA = "\033[105m";
	constexpr Code BG_BRIGHT_CYAN    = "\033[106m";
	constexpr Code BG_BRIGHT_WHITE   = "\033[107m";

	// 256-color helper (optional use)
	constexpr Code FG_256 = "\033[38;5;";
	constexpr Code BG_256 = "\033[48;5;";
}
