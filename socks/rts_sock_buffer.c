#include "rts_sock_buffer.h"
#include "rts_os.h"

#include <stdlib.h>
#include <string.h>

rts_sock_buffer_t rts_sock_buffer_create(int length, rts_sock_os_t* os, rts_sock_t sock) {
	rts_sock_buffer_t buffer;
	buffer.os = os;
	buffer.sock = sock;

	buffer.length = sizeof(char) * length;
	buffer.data = (char*)malloc(buffer.length);
	
	rts_sock_buffer_clear(&buffer);
	
	return buffer;
}

void rts_sock_buffer_copy_from(const void* source, int source_length, rts_sock_buffer_t* dest, bool must_null_terminate) {

	// In case source is larger
	int byte_quantity = RTS_MIN(source_length, dest->length);

	// If the data we're obtaining is smaller than ourselves, blank ourselves first
	// so the remaining byte diff is just empty padding
	if (byte_quantity < dest->length) {
		memset(dest->data, 0, dest->length);
	}

	memcpy(dest->data, source, byte_quantity);

	if (must_null_terminate) {
		rts_sock_buffer_null_terminate(dest);
	}
}

void rts_sock_buffer_clear(rts_sock_buffer_t* buffer) {
	memset(buffer->data, 0, buffer->length);

	buffer->rw.write_index = 0;
	buffer->rw.read_index = 0;
}

void rts_sock_buffer_destroy(rts_sock_buffer_t* buffer) {
	free(buffer->data);
	buffer->data = NULL;
	buffer->length = 0;
}

bool rts_sock_buffer_recv(rts_sock_buffer_t* buffer, rts_eh_t* eh, int* bytes_read) {
	return buffer->os->recv(eh, buffer->sock, buffer->data, buffer->length, bytes_read);
}

bool rts_sock_buffer_recv_until_full(rts_sock_buffer_t* buffer, rts_eh_t* eh, bool must_null_terminate, bool* disconnect_flag) {
	int bytes_read = 0;

	rts_sock_buffer_clear(buffer);

	do {
		RTS_ASSERT(eh, buffer->rw.write_index >= 0);
		RTS_ASSERT(eh, buffer->rw.write_index < buffer->length);

		if (buffer->rw.write_index != 0) {
			rts_info(eh, "Multi-part receive");
		}

		bool ok = buffer->os->recv(
			eh,
			buffer->sock,
			buffer->data + buffer->rw.write_index,
			buffer->length - buffer->rw.write_index,
			&bytes_read);

		if (!ok) {
			rts_warning(eh, "Cannot receive until buffer full: Receive returned error");
			return false;
		}

		if (bytes_read == 0) {
			// Working normally but client is gone
			*disconnect_flag = true;
			return true;
		}

		buffer->rw.write_index += bytes_read;

	} while (buffer->rw.write_index < buffer->length);

	if (must_null_terminate) {
		rts_sock_buffer_null_terminate(buffer);
	}
}

bool rts_sock_buffer_send_until_finished(rts_sock_buffer_t* buffer, rts_eh_t* eh) {
	int bytes_sent = 0;

	buffer->rw.read_index = 0;

	do {
		RTS_ASSERT(eh, buffer->rw.read_index >= 0);
		RTS_ASSERT(eh, buffer->rw.read_index < buffer->length);

		if (buffer->rw.read_index != 0) {
			rts_info(eh, "Multi-part send");
		}
		
		bool ok = buffer->os->send(
			eh,
			buffer->sock,
			buffer->data + buffer->rw.read_index,
			buffer->length - buffer->rw.read_index,
			&bytes_sent);

		if (!ok) {
			rts_warning(eh, "Cannot send until buffer full: Send returned error");
			return false;
		}

		buffer->rw.read_index += bytes_sent;

	} while (buffer->rw.read_index < buffer->length);
}

void rts_sock_buffer_null_terminate(rts_sock_buffer_t* buffer) {
	if (buffer->data[buffer->length - 1] != 0) {
		// Force null termination of string
		buffer->data[buffer->length - 1] = 0;
	}
}