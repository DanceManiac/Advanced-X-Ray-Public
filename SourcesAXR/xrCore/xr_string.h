#pragma once
#include <string>
#include "xalloc.h"

// string(char)
typedef std::basic_string<char, std::char_traits<char>, xalloc<char>> xr_string;