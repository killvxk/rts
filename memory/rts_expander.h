#pragma once
#include "rts_eh.h"
#include <stdbool.h>

#define RTS_EXPANDER_DEBUG

// Policy for growing the buffer - takes current size in bytes 
// and returned new size in bytes. minimum_total_bytes indicates how large the buffer 
// must be, at minimum, to satisfy the data being added to it at the point the grow
// was requested.
//
// Implementations should not ever return a size smaller than the minimum
typedef int rts_expander_grow_handler(int current_size_bytes, int minimum_total_size_bytes);

// Destructor for an individual item
typedef void rts_expander_destruct_handler(void* item, void* userdata);

typedef struct {

	// Assumed size of each item
	int assumed_item_size;

	// Arbitrary user data
	void* userdata;

	// Run every time an item is removed
	bool on_item_remove;

	rts_expander_destruct_handler* handler;

} rts_expander_destructor_t;

// Expanding buffer
typedef struct rts_expander_struct {

	char* data;

	// Total size of the buffer in bytes - not anything 
	// to do with where in the buffer has been written etc.
	// JUST the slab of memory
	int total_buffer_size_bytes;

	rts_expander_grow_handler* grow_policy;

	// Destructor info
	rts_expander_destructor_t destructor;

	// Total quantity of items written. This value is only useful if the 
	// buffer contains values of uniform type
	int items;

} rts_expander_t;

rts_expander_t* rts_expander_create(rts_eh_t* eh, int size_in_bytes);

void rts_expander_register_destructor(rts_expander_t* e, int item_size, void* userdata, bool on_remove, rts_expander_destruct_handler* handler);

void rts_expander_clear(rts_expander_t* e);

void rts_expander_grow_now(rts_eh_t* eh, rts_expander_t* e, int minimum_total_size_bytes);

void rts_expander_grow_specific(rts_eh_t* eh, rts_expander_t* e, int demand_total_size_bytes);

// Add an item. Item-level tracking only works if all items are of uniform size
void rts_expander_add_item(rts_eh_t* eh, rts_expander_t* e, void* data, int size);

// Get an item, by its index (not byte offset) - see the items member.
// Item-level tracking only works if all items are of uniform size
void rts_expander_get_item(rts_eh_t* eh, rts_expander_t* e, int index, void* read_into, int size);

// Remove an item by its index (not byte offset)
// Item-level tracking only works if all items are of uniform size - removal especially so!
void rts_expander_remove_item(rts_eh_t* eh, rts_expander_t* e, int index, int size);

// Write data to the buffer. Will dynamically expand if the data does not fit
void rts_expander_write(rts_eh_t* eh, rts_expander_t* e, int index, void* data, int size);

// Write data to the buffer without any attempt to grow. 
// It is expected the caller has performed rts_expander_write_will_grow etc. to determine if
// the data will fit
void rts_expander_write_no_grow(rts_eh_t* eh, rts_expander_t* e, int index, void* data, int size);

// Whether a write to the given index will trigger growth of the buffer. 
// margin_bytes is set to how much the buffer will grow *by* to accomodate the data being written
bool rts_expander_write_will_grow(rts_eh_t* eh, rts_expander_t* e, int index, int size, int* margin_bytes);

// Read data from the buffer. If the request is larger than the buffer, the buffer will panic
void rts_expander_read(rts_eh_t* eh, rts_expander_t* e, int index, void* read_into, int size);

void rts_expander_destroy(rts_eh_t* eh, rts_expander_t* e, bool destruct_items, void* destructor_info);