#pragma once

#if defined(_WIN64) || defined(_WIN32)
	#define RTS_PLAT_WINDOWS
#endif

#if defined(__linux__)

	// Temp
	#define RTS_PLAT_UNIX
#endif