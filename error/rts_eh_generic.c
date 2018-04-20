#include "rts_eh.h"
#include <stdio.h>

#include <stdio.h>
#include <stdlib.h>

#include "rts_windows.h"

static void debugger(const char* format, va_list args) {

#ifdef RTS_PLAT_WINDOWS
	char buffer[512];

	vsnprintf_s(buffer, 512, _TRUNCATE, format, args);

	OutputDebugString(buffer);
	OutputDebugString("\n");
#endif
}

// Generic stdout logging/error info
static void panic(const char* format, va_list args) {
	printf("PANIC\t");
	vprintf(format, args);
	printf("\n");

	debugger(format, args);

#ifdef RTS_PLAT_WINDOWS
	__debugbreak();
#endif

	abort();
}

static void warning(const char* format, va_list args) {
	printf("WARNING\t");
	vprintf(format, args);
	printf("\n");

	debugger(format, args);
}

static void info(const char* format, va_list args) {
	vprintf(format, args); 
	printf("\n");

	debugger(format, args);
}

rts_eh_t rts_eh_create_generic() {
	rts_eh_t eh;
	eh.panic = &panic;
	eh.info = &info;
	eh.warning = &warning;

	return eh;
}