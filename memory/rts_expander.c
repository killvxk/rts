#include "rts_expander.h"
#include "rts_alloc.h"
#include <string.h>

int default_grow_policy(int current_size_bytes, int minimum_total_size_bytes) {

	int typical = current_size_bytes * 2;

	if (typical > minimum_total_size_bytes) {
		return typical;
	} else {

		// Dubious
		return minimum_total_size_bytes * 4;
	}
}

rts_expander_t* rts_expander_create(rts_eh_t* eh, int size_in_bytes) {

#ifdef RTS_EXPANDER_DEBUG
	rts_info(eh, "Expanding buffer create %db", size_in_bytes);
#endif

	rts_expander_t* e = (rts_expander_t*)rts_alloc(0, sizeof(rts_expander_t));
	e->data = (char*)rts_alloc(0, size_in_bytes);
	e->total_buffer_size_bytes = size_in_bytes;
	e->grow_policy = &default_grow_policy;
	rts_expander_clear(e);
	return e;
}

void rts_expander_clear(rts_expander_t* e) {
	memset(e->data, 0, e->total_buffer_size_bytes);
	e->items = 0;
}

void rts_expander_destroy(rts_eh_t* eh, rts_expander_t* e) {

#ifdef RTS_EXPANDER_DEBUG
	rts_info(eh, "Destroy expanding buffer size %db", e->total_buffer_size_bytes);
#endif 

	rts_free(e->data);
	
	e->items = 0;
	e->data = NULL;
	e->total_buffer_size_bytes = 0;

	rts_free(e);
}

void rts_expander_grow_now(rts_eh_t* eh, rts_expander_t* e, int minimum_total_size_bytes) {
	rts_expander_grow_specific(eh, e, e->grow_policy(e->total_buffer_size_bytes, minimum_total_size_bytes));
}

void rts_expander_grow_specific(rts_eh_t* eh, rts_expander_t* e, int demand_total_size_bytes) {

	if (demand_total_size_bytes == e->total_buffer_size_bytes) {
		return;
	}

#ifdef RTS_EXPANDER_DEBUG
	rts_info(eh, "Expanding buffer grow demand %db -> %db", e->total_buffer_size_bytes, demand_total_size_bytes);
#endif
	
	// Do not permit shrink
	RTS_ASSERT(eh, demand_total_size_bytes > e->total_buffer_size_bytes);

	e->data = realloc(e->data, demand_total_size_bytes);

	RTS_ASSERT(eh, e->data != NULL);
	
	// We need to clear the difference between the old buffer and the new buffer (starting at the end of the old size)
	memset(e->data + e->total_buffer_size_bytes, 0, demand_total_size_bytes - e->total_buffer_size_bytes);

	e->total_buffer_size_bytes = demand_total_size_bytes;
}

void rts_expander_write(rts_eh_t* eh, rts_expander_t* e, int offset, void* data, int size) {
	RTS_ASSERT(eh, offset >= 0 && offset < e->total_buffer_size_bytes);

	int end_offset = offset + size;
	
	if (end_offset >= e->total_buffer_size_bytes) {

		// How many bytes we're too big by
		int margin_size_bytes = end_offset - (e->total_buffer_size_bytes - 1); // -1 turns size -> valid index

		rts_expander_grow_now(eh, e, e->total_buffer_size_bytes + margin_size_bytes);
	}

	// Should now definitely fit
	RTS_ASSERT(eh, (offset + size) < e->total_buffer_size_bytes);

	memcpy(e->data + offset, data, size);	
}

void rts_expander_read(rts_eh_t* eh, rts_expander_t* e, int offset, void* read_into, int size) {
	RTS_ASSERT(eh, offset >= 0 && offset < e->total_buffer_size_bytes);

	if (offset + size >= e->total_buffer_size_bytes) {
		// We don't grow for reads
		rts_panic(eh, "Read from offset %d size %db exceeds buffer size %d", offset, size, e->total_buffer_size_bytes);
	}

	memcpy(read_into, e->data + offset, size);
}

void rts_expander_add_item(rts_eh_t* eh, rts_expander_t* e, void* data, int size) {
	rts_expander_write(eh, e, e->items * size, data, size);
	e->items++;
}

void rts_expander_get_item(rts_eh_t* eh, rts_expander_t* e, int index, void* read_into, int size) {
	RTS_ASSERT(eh, index >= 0 && index < e->items);

	rts_expander_read(eh, e, index * size, read_into, size);
}

void rts_expander_remove_item(rts_eh_t* eh, rts_expander_t* e, int index, int size) {
	RTS_ASSERT(eh, index >= 0 && index < e->items);

	RTS_ASSERT(eh, index * size >= 0 && index * size < e->total_buffer_size_bytes);
	RTS_ASSERT(eh, ((index * size) + size) < e->total_buffer_size_bytes);

	// Make the data to be removed easily recognised
	memset(e->data + (index * size), 126, size);
		
	memmove(
		e->data + (index * size), // Dest = what we just removed's STARTING point

		e->data + (index * size) + size, // Src = what is AFTER the item we are removing

		(e->total_buffer_size_bytes - ((index * size) + size))); // Size is the length betwen the end of the
															   // item we removed and the end of the buffer

	e->items--;
	
	// There is now garbage at the end of the buffer (Maybe? Docs do not state) - zero out between the end of the last 
	// and the total size of the buffer

	int last_item = e->items - 1;

	memset(
		e->data + ((last_item * size) + size),  // Last item's END point
		0,
		(e->total_buffer_size_bytes - ((last_item * size) + size))); // length between the end of the last item 
																	// and the end of the buffer


}