#pragma once
#include <stdlib.h>

void* rts_alloc(int mode, unsigned int length);

void rts_free(void* mem);