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

	// Always permit reuse of address
	int enable = 1;

	if (setsockopt(sock._value, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(enable)) != 0) {
		// Sets errno on failure
		rts_panic_unix_errno(eh, "Failed to enable address reuse before bind");
		freeaddrinfo(result);
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

bool socket_listen(rts_eh_t* eh, rts_sock_t sock) {

	if (listen(sock._value, SOMAXCONN) == 0) {
		rts_info(eh, "Listening started on socket %d", sock._value);
		return true;
	} else {
		// Sets errno on failure
		rts_panic_unix_errno(eh, "Failed to start listening on socket");
		return false;
	}
}

bool socket_accept(rts_eh_t* eh, rts_sock_t listener, rts_sock_t* new_client) {

	int client = accept(listener._value, NULL, NULL);

	if (client < 0) {

		int current_errno = errno;

		rts_warning(eh, "Accept of incoming connection produced invalid socket. errno is %d", current_errno);

		new_client->_value = -1;
		return false;
	} else {
		new_client->_value = client;
		return true;
	}
}

bool socket_recv(rts_eh_t* eh, rts_sock_t sock, char* buffer, int buffer_length, int* bytes_read) {

	int result = recv(sock._value, buffer, buffer_length, 0); // TODO: PEEK should be separate

	if (result < 0) {

		*bytes_read = -1;

		int current_errno = errno;

		rts_warning(eh, "Receive from socket failed. errno is %d", current_errno);

		return false;
	}
	else {

		*bytes_read = result;
		return true;
	}
}

void rts_sock_linux_attach(rts_sock_os_t* os) {
	os->open = &socket_open;
	os->close = &socket_close;
	os->bind = &socket_bind;
	os->listen = &socket_listen;
	os->accept = &socket_accept;
	os->recv = &socket_recv;
}

#endif