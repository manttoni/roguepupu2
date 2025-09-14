#include <ncurses.h>
#include "ColorPair.hpp"
#include "UI.hpp"

ColorPair::ColorPair() : id(0) {} // doesnt "init_pair"
ColorPair::ColorPair(const short id, const Color& fg, const Color& bg) : id(id), fg(fg), bg(bg)
{} // THIS HAS TO BE ALREADY INITIALIZED, LET UI DO IT

