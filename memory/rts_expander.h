#pragma once
#include "rts_eh.h"

#define RTS_EXPANDER_DEBUG

// Policy for growing the buffer - takes current size in bytes 
// and returned new size in bytes. minimum_total_bytes indicates how large the buffer 
// must be, at minimum, to satisfy the data being added to it at the point the grow
// was requested.
//
// Implementations should not ever return a size smaller than the minimum
typedef int rts_expander_grow_handler(int current_size_bytes, int minimum_total_size_bytes);

// Expanding buffer
typedef struct {

	char* data;

	// Total size of the buffer in bytes - not anything 
	// to do with where in the buffer has been written etc.
	// JUST the slab of memory
	int total_buffer_size_bytes;

	rts_expander_grow_handler* grow_policy;

	// Total quantity of items written. This value is only useful if the 
	// buffer contains values of uniform type
	int items;

} rts_expander_t;

rts_expander_t* rts_expander_create(rts_eh_t* eh, int size_in_bytes);

void rts_expander_clear(rts_expander_t* e);

void rts_expander_grow_now(rts_eh_t* eh, rts_expander_t* e, int minimum_total_size_bytes);

void rts_expander_grow_specific(rts_eh_t* eh, rts_expander_t* e, int demand_total_size_bytes);

// Add an item. Item-level tracking only works if all items are of uniform size
void rts_expander_add_item(rts_eh_t* eh, rts_expander_t* e, void* data, int size);

// Get an ite, by its index (not byte offset) - see the items member.
// Item-level tracking only works if all items are of uniform size
void rts_expander_get_item(rts_eh_t* eh, rts_expander_t* e, int index, void* read_into, int size);


// Write data to the buffer. Will dynamically expand if the data does not fit
void rts_expander_write(rts_eh_t* eh, rts_expander_t* e, int offset, void* data, int size);

// Read data from the buffer. If the request is larger than the buffer, the buffer will panic
void rts_expander_read(rts_eh_t* eh, rts_expander_t* e, int offset, void* read_into, int size);

void rts_expander_destroy(rts_eh_t* eh, rts_expander_t* e);