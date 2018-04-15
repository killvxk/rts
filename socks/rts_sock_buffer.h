#pragma once
#include "rts_sock_os.h"

// Read/write info for the socket recv buffer
typedef struct {
	int write_index;
	int read_index;
} rt_sock_buffer_rw_t;

// Wrapper around a buffer designed to assist with recv calls
typedef struct {

	char* data;

	int length;

	rts_sock_os_t* os;

	rts_sock_t sock;

	rt_sock_buffer_rw_t rw;

} rts_sock_buffer_t;

// Create a buffer of a given length
rts_sock_buffer_t rts_sock_buffer_create(int length, rts_sock_os_t* os, rts_sock_t sock);

void rts_sock_buffer_copy_from(const void* source, int source_length, rts_sock_buffer_t* dest, bool must_null_terminate);

// Completely clear the buffer
void rts_sock_buffer_clear(rts_sock_buffer_t* buffer);

// Destroy the buffer
void rts_sock_buffer_destroy(rts_sock_buffer_t* buffer);

bool rts_sock_buffer_recv(rts_sock_buffer_t* buffer, rts_eh_t* eh, int* bytes_read);

// Perform recvs until the buffer is full. Does NOT use socket level MSG_WAITALL - purely
// managed by the buffer itself receiving in small fragments.
bool rts_sock_buffer_recv_until_full(rts_sock_buffer_t* buffer, rts_eh_t* eh, bool must_null_terminate, bool* disconnect_flag);

// Keep performing sends until the entire buffer is sent
bool rts_sock_buffer_send_until_finished(rts_sock_buffer_t* buffer, rts_eh_t* eh);

// Ensure the buffer is null terminated if it isn't already
void rts_sock_buffer_null_terminate(rts_sock_buffer_t* buffer);