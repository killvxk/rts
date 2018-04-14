#include "rts_sock_linux.h"

#ifdef RTS_PLAT_LINUX

#include <stdlib.h>
#include <string.h>

rts_sock_t socket_open(rts_eh_t* eh) {

	rts_sock_t sock;
	sock._value = socket(AF_INET6, SOCK_STREAM, 0);

	if (sock._value < 0) {
		rts_panic_unix_errno(eh, "Open generated invalid socket");
	}

	return sock;
}

void socket_close(rts_eh_t* eh, rts_sock_t sock) {
	
	if (close(sock._value) != 0) {
		rts_panic_unix_errno(eh, "Failed to close socket!");
	}
}

bool socket_bind(rts_eh_t* eh, rts_sock_t sock, int port) {
	struct addrinfo hint_struct;
	memset(&hint_struct, 0, sizeof(struct addrinfo));

	// TODO: Forcing this to IPV6 because docker for ex. prefers v4 when given choice
	// that isn't directly connected to socket in use
	hint_struct.ai_family = AF_INET6;
	hint_struct.ai_socktype = SOCK_STREAM;
	hint_struct.ai_flags = AI_PASSIVE;

	struct addrinfo* result;

	char* service_name = rts_sock_parse_port(port);

	int ret = getaddrinfo(NULL, service_name, &hint_struct, &result);

	free(service_name);

	if (ret != 0) {
		// Returns EAI error codes which could be translated with gai_strerror
		rts_panic(eh, "Failed to get address info from current host, code: %d", ret);
		return false;
	}

	if (bind(sock._value, result->ai_addr, result->ai_addrlen) == 0) {
		freeaddrinfo(result);
		rts_info(eh, "Bound socket %d to port %d", sock._value, port);
		return true;
	} else {

		// Sets errno on failure
		rts_panic_unix_errno(eh, "Failed to bind socket to port");
		freeaddrinfo(result);
		return false;
	}
}

void rts_sock_linux_attach(rts_sock_os_t* sock) {
	sock->open = &socket_open;
	sock->close = &socket_close;
	sock->bind = &socket_bind;
}

#endif