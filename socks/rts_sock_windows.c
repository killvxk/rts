#include "rts_os.h"

#ifdef  RTS_PLAT_WINDOWS
#include "rts_sock_windows.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool winsock_start(rts_eh_t* eh) {
	WSADATA wsa_data;

	int ret = WSAStartup(MAKEWORD(2, 2), &wsa_data);

	if (ret == 0) {
		rts_info(eh, "Winsock started OK");
		return true;
	} else {

		// As per MSDN, the return value *IS* the error code
		rts_panic(eh, "Winsock failed to start with error code %d", ret);
		return false;
	}
}

void winsock_stop(rts_eh_t* eh) {
	if (WSACleanup() == 0) {
		rts_info(eh, "Winsock stopped OK");
	} else {
		rts_panic_winsock_error(eh, "Winsock failed to cleanup");
	}
}

rts_sock_t socket_open(rts_eh_t* eh) {
	rts_sock_t sock;
	sock._value = socket(AF_INET6, SOCK_STREAM, 0);

	if (sock._value == INVALID_SOCKET) {
		rts_panic_winsock_error(eh, "Open generated invalid socket");
	}

	return sock;
}

void socket_close(rts_eh_t* eh, rts_sock_t sock) {
	if (closesocket(sock._value) != 0) {
		rts_panic_winsock_error(eh, "Failed to close socket");
	}
}

bool socket_bind(rts_eh_t* eh, rts_sock_t sock, int port) {
	ADDRINFOA hint_struct;
	memset(&hint_struct, 0, sizeof(ADDRINFOA));

	hint_struct.ai_family = AF_UNSPEC;  
	hint_struct.ai_socktype = SOCK_STREAM;
	hint_struct.ai_flags = AI_PASSIVE;

	ADDRINFOA* result;

	char* service_name = rts_sock_parse_port(port);

	int ret = getaddrinfo(NULL, service_name, &hint_struct, &result);
	
	free(service_name);

	if (ret != 0) {
		// Returns WSA error code directly, doesn't set last error
		rts_panic(eh, "Failed to get address info from current host, code: %d", ret);

		return false;
	}

	if (bind(sock._value, result->ai_addr, result->ai_addrlen) == 0) {
		
		freeaddrinfo(result);

		rts_info(eh, "Bound socket %d to port %d", sock._value, port);
		return true;

	} else {
		// Sets a specific WSA error
		rts_panic_winsock_error(eh, "Failed to bind socket to port");
		freeaddrinfo(result);
		return false;
	}
}

void rts_sock_windows_attach(rts_sock_os_t* sock) {
	sock->global_start = &winsock_start;
	sock->global_stop = &winsock_stop;

	sock->open = &socket_open;
	sock->close = &socket_close;

	sock->bind = &socket_bind;
}


#endif