#pragma once

// Opaque for a socket
typedef struct {

	// Actual socket value. Doesn't need to be opaque per platform since 
	// behaves the same across windows and unix
	int _value;

} rts_sock_t;

