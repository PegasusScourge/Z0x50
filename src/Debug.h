#pragma once

#include <stdio.h>

#define DEBUG_OUT stderr

#ifdef _DEBUG
#define debug_printf(fmt, ...) fprintf(DEBUG_OUT, "[DEBUG] %s msg --> ", __FUNCTION__); fprintf(DEBUG_OUT, fmt, __VA_ARGS__);
#else
#define debug_printf(fmt, ...)
#endif
