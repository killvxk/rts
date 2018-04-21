#pragma once 
#include "rts_expander.h"

// Circular buffer
//
// This buffer is totally *unsafe* - it does not track the overlap of read/write
// or where valid items begin or end. It simply loops around on read/write
typedef struct {

	rts_expander_t* buffer;

	int write_ptr;

	int read_ptr;

} rts_circular_t;

rts_circular_t* rts_circular_create(rts_eh_t* eh, int size_in_bytes);

void rts_circular_destroy(rts_eh_t* eh, rts_circular_t* c);

// Returns true if the write looped around
bool rts_circular_write(rts_eh_t* eh, rts_circular_t* c, void* data, int size);

// Returns true if the read looped around
bool rts_circular_read(rts_eh_t* eh, rts_circular_t* c, void* read_into, int size);

// Reset the read head to the beginning
void rts_circular_reset_read(rts_circular_t* c);

// Resets the write head to the beginning
void rts_circular_reset_write(rts_circular_t* c);