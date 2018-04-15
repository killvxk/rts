#include "rts_sock_buffer.h"

#include <stdlib.h>
#include <string.h>

rts_sock_buffer_t rts_sock_buffer_create(int length, rts_sock_os_t* os, rts_sock_t sock) {
	rts_sock_buffer_t buffer;
	buffer._os = os;
	buffer._sock = sock;

	buffer._length = sizeof(char) * length;
	buffer._data = (char*)malloc(buffer._length);
	
	rts_sock_buffer_clear(&buffer);


	return buffer;
}

void rts_sock_buffer_clear(rts_sock_buffer_t* buffer) {
	memset(buffer->_data, 0, buffer->_length);

	buffer->_rw.write_index = 0;
}

void rts_sock_buffer_destroy(rts_sock_buffer_t* buffer) {
	free(buffer->_data);
	buffer->_data = NULL;
	buffer->_length = 0;
}

bool rts_sock_buffer_recv(rts_sock_buffer_t* buffer, rts_eh_t* eh, int* bytes_read) {
	return buffer->_os->recv(eh, buffer->_sock, buffer->_data, buffer->_length, bytes_read);
}

bool rts_sock_buffer_recv_until_full(rts_sock_buffer_t* buffer, rts_eh_t* eh, bool must_null_terminate, bool* disconnect_flag) {
	int bytes_read = 0;

	rts_sock_buffer_clear(buffer);

	do {
		RTS_ASSERT(eh, buffer->_rw.write_index >= 0);
		RTS_ASSERT(eh, buffer->_rw.write_index < buffer->_length);

		bool ok = buffer->_os->recv(
			eh,
			buffer->_sock,
			buffer->_data + buffer->_rw.write_index,
			buffer->_length - buffer->_rw.write_index,
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

		buffer->_rw.write_index += bytes_read;

	} while (buffer->_rw.write_index < buffer->_length);

	if (must_null_terminate && buffer->_data[buffer->_length - 1] != 0) {
		// Force null termination of string
		buffer->_data[buffer->_length - 1] = 0;
	}
}