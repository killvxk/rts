#include "rts_eh.h"
#include <stdio.h>

#include <stdio.h>
#include <stdlib.h>

// Generic stdout logging/error info

static void panic(const char* format, va_list args) {
	printf("PANIC\t");
	vprintf(format, args);
	printf("\n");
	abort();
}

static void warning(const char* format, va_list args) {
	printf("WARNING\t");
	vprintf(format, args);
	printf("\n");
}

static void info(const char* format, va_list args) {
	vprintf(format, args); 
	printf("\n");
}

rts_eh_t rts_eh_create_generic() {
	rts_eh_t eh;
	eh.panic = &panic;
	eh.info = &info;
	eh.warning = &warning;

	return eh;
}