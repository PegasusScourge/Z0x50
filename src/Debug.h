#pragma once

#ifdef _DEBUG
#define __DEBUG 1
#else
#define __DEBUG 0
#endif

#define DEBUG_printf(fmt, ...) do { if (__DEBUG) { fprintf(stderr, "DEBUG: fn %s, ", __FUNCTION__); fprintf(stderr, fmt, __VA_ARGS__); } } while (0)
