#include "rts_alloc.h"

#include <stdlib.h>
#include <string.h>

// Temporary. Mode reserved for indicating probably specific pool or allocation type

void* rts_alloc(int mode, unsigned int length) {
	return malloc(length);
}

void rts_free(void* mem) {
	free(mem);
}