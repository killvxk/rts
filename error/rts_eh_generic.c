#include "rts_eh.h"
#include <stdio.h>

// Generic stdout logging/error info

static void panic(const char* format, va_list args) {
	printf("PANIC\t");
	vprintf(format, args);
	printf("\n");
	abort();
}

static void info(const char* format, va_list args) {
	vprintf(format, args); 
	printf("\n");
}

rts_eh_t rts_eh_create_generic() {
	rts_eh_t eh;
	eh.panic = &panic;
	eh.info = &info;

	return eh;
}