#include "rts_expander.h"
#include "rts_alloc.h"
#include <string.h>

static int default_grow_policy(int current_size_bytes, int minimum_total_size_bytes) {

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

	e->destructor.assumed_item_size = 0;
	e->destructor.userdata = NULL;
	e->destructor.handler = NULL;
	e->destructor.on_item_remove = false;

	rts_expander_clear(e);
	return e;
}

void rts_expander_clear(rts_expander_t* e) {
	memset(e->data, 0, e->total_buffer_size_bytes);
	e->items = 0;
}

void rts_expander_register_destructor(rts_expander_t* e, int item_size, void* userdata, bool on_remove, rts_expander_destruct_handler* handler) {
	e->destructor.assumed_item_size = item_size;
	e->destructor.userdata = userdata;
	e->destructor.handler = handler;
	e->destructor.on_item_remove = on_remove;
}

void rts_expander_destroy(rts_eh_t* eh, rts_expander_t* e, bool destruct_items, void* destructor_info) {

#ifdef RTS_EXPANDER_DEBUG
	rts_info(eh, "Destroy expanding buffer size %db", e->total_buffer_size_bytes);
#endif 

	if (destruct_items) {
		if (e->destructor.handler != NULL) {

			for (int i = 0; i < e->items; i++) {
				e->destructor.handler( (char*)e->data + (i * e->destructor.assumed_item_size), e->destructor.userdata);
			}
		}
	}

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

bool rts_expander_write_will_grow(rts_eh_t* eh, rts_expander_t* e, int index, int size, int* margin_bytes) {
	
	// The index is allowed to be one-over the total size of the buffer, since such 
	// a thing is what prompts us to grow
	RTS_ASSERT(eh, index >= 0 && index <= e->total_buffer_size_bytes);

	int end_index = (index + (size - 1));

	if (end_index > (e->total_buffer_size_bytes - 1)) {

		// How many bytes we're too big by
		*margin_bytes = end_index - (e->total_buffer_size_bytes - 1); // -1 turns size -> valid index

		RTS_ASSERT(eh, *margin_bytes != 0);

		return true;
	} else {
		*margin_bytes = 0;
		return false;
	}
}

void rts_expander_write_no_grow(rts_eh_t* eh, rts_expander_t* e, int index, void* data, int size) {
	RTS_ASSERT(eh, index >= 0 && index < e->total_buffer_size_bytes);
	RTS_ASSERT(eh, (index + (size - 1)) < e->total_buffer_size_bytes);

	memcpy(e->data + index, data, size);
}

void rts_expander_write(rts_eh_t* eh, rts_expander_t* e, int index, void* data, int size) {
	
	// Index is allowed to be = total size in bytes (i.e. buffer overflow)
	// because this *prompts* us to grow the buffer
	RTS_ASSERT(eh, index >= 0 && index <= e->total_buffer_size_bytes);

	if (size == 0) {
		return;
	}

	int margin_size_bytes = 0;

	if (rts_expander_write_will_grow(eh, e, index, size, &margin_size_bytes)) {
		rts_expander_grow_now(eh, e, e->total_buffer_size_bytes + margin_size_bytes);
	}

	rts_expander_write_no_grow(eh, e, index, data, size);
}

void rts_expander_read(rts_eh_t* eh, rts_expander_t* e, int index, void* read_into, int size) {
	RTS_ASSERT(eh, index >= 0 && index < e->total_buffer_size_bytes);

	if ((index + (size - 1)) >= e->total_buffer_size_bytes) {
		// We don't grow for reads
		rts_panic(eh, "Read from offset %d size %db exceeds buffer size %d", index, size, e->total_buffer_size_bytes);
	}

	memcpy(read_into, e->data + index, size);
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

	// Destroy the item itself / its resources if configured to do so
	if (e->destructor.on_item_remove && e->destructor.handler != NULL) {
		RTS_ASSERT(eh, e->destructor.assumed_item_size == size);
		e->destructor.handler((char*)e->data + (index * size), e->destructor.userdata);
	}

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