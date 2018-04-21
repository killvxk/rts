#include "rts_circular.h"
#include "rts_alloc.h"

static int no_grow(int current_size_bytes, int minimum_total_size_bytes) {
	rts_panic(NULL, "Circular buffer should never grow! (%db -> %db)", current_size_bytes, minimum_total_size_bytes);

	return -1;
}

rts_circular_t* rts_circular_create(rts_eh_t* eh, int size_in_bytes) {
	rts_circular_t* c = rts_alloc(0, sizeof(rts_circular_t));
	c->buffer = rts_expander_create(eh, size_in_bytes);
	c->buffer->grow_policy = &no_grow;
	c->write_ptr = 0;
	c->read_ptr = 0;
	return c;
}

void rts_circular_destroy(rts_eh_t* eh, rts_circular_t* c) {
	rts_expander_destroy(eh, c->buffer, false, NULL);
	rts_free(c);
}

bool rts_circular_write(rts_eh_t* eh, rts_circular_t* c, void* data, int size) {

	int margin_bytes = 0;

	// We need to prevent buffer growth (and loop around instead)
	if (rts_expander_write_will_grow(eh, c->buffer, c->write_ptr, size, &margin_bytes)) {

		int write_now = size - margin_bytes;

		if (write_now != 0) {

			// Partial write to end and loop back
			rts_expander_write_no_grow(eh, c->buffer, c->write_ptr, data, write_now);
		}

		rts_expander_write(eh, c->buffer, 0, ((char*)data + write_now), margin_bytes);

		c->write_ptr = margin_bytes;

		return true;

	} else {
		// Can perform a full write
		rts_expander_write(eh, c->buffer, c->write_ptr, data, size);
		c->write_ptr += size;

		return false;
	}
}

bool rts_circular_read(rts_eh_t* eh, rts_circular_t* c, void* read_into, int size) {
	int margin_bytes = 0;

	if (c->read_ptr + (size - 1) > (c->buffer->total_buffer_size_bytes - 1)) {

		// This read takes us beyond the end of the buffer
		//
		// How much of an over-read there would have been
		int margin_bytes = (c->read_ptr + (size - 1)) - (c->buffer->total_buffer_size_bytes - 1);

		int read_now = size - margin_bytes;

		if (read_now != 0) {
			rts_expander_read(eh, c->buffer, c->read_ptr, read_into, read_now);
		}

		rts_expander_read(eh, c->buffer, 0, ((char*)read_into + read_now), margin_bytes);

		c->read_ptr = margin_bytes;

		return true;

	} else {

		// Normal complete read
		rts_expander_read(eh, c->buffer, c->read_ptr, read_into, size);
		c->read_ptr += size;

		return false;
	}
}

void rts_circular_reset_read(rts_circular_t* c) {
	c->read_ptr = 0;
}

void rts_circular_reset_write(rts_circular_t* c) {
	c->write_ptr = 0;
}