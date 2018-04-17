#include "rts_os.h"

#ifdef  RTS_PLAT_WINDOWS
#include "rts_sock_windows.h"

#include <stdio.h>
#include "rts_alloc.h"

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
	sock.value = socket(AF_INET, SOCK_STREAM, 0);

	if (sock.value == INVALID_SOCKET) {
		rts_panic_winsock_error(eh, "Open generated invalid socket");
	}

	return sock;
}

void socket_close(rts_eh_t* eh, rts_sock_t sock) {
	if (closesocket(sock.value) != 0) {
		rts_panic_winsock_error(eh, "Failed to close socket");	
	}
}

bool socket_bind(rts_eh_t* eh, rts_sock_t sock, int port) {
	ADDRINFOA hint_struct;
	memset(&hint_struct, 0, sizeof(ADDRINFOA));

	hint_struct.ai_family = AF_INET;  
	hint_struct.ai_socktype = SOCK_STREAM;
	hint_struct.ai_flags = AI_PASSIVE;
	
	ADDRINFOA* result;

	char* service_name = rts_sock_parse_port(port);

	int ret = getaddrinfo(NULL, service_name, &hint_struct, &result);
		
	rts_free(service_name);

	if (ret != 0) {
		// Returns WSA error code directly, doesn't set last error
		rts_panic(eh, "Failed to get address info from current host, code: %d", ret);
		return false;
	}

	// Always permit reuse of address
	int enable = 1;

	if (setsockopt(sock.value, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(enable)) != 0) {
		// Sets a specific WSA error
		rts_panic_winsock_error(eh, "Failed to enable address reuse before bind");
		freeaddrinfo(result);
		return false;
	}

	if (bind(sock.value, result->ai_addr, result->ai_addrlen) == 0) {		
		freeaddrinfo(result);
		rts_info(eh, "Bound socket %d to port %d", sock.value, port);
		return true;
	} else {
		// Sets a specific WSA error
		rts_panic_winsock_error(eh, "Failed to bind socket to port");
		freeaddrinfo(result);
		return false;
	}
}

bool socket_listen(rts_eh_t* eh, rts_sock_t sock) {

	if (listen(sock.value, SOMAXCONN) == 0) {		
		rts_info(eh, "Listening started on socket %d", sock.value);
		return true;
	} else {
		// Sets a specific WSA error
		rts_panic_winsock_error(eh, "Failed to start listening on socket");
		return false;
	}
}

bool socket_accept(rts_eh_t* eh, rts_sock_t listener, rts_sock_t* new_client) {

	// TODO: Take the client addr info (useful)
	SOCKET client = accept(listener.value, NULL, NULL);

	if (client == INVALID_SOCKET) {

		int err = WSAGetLastError();

		// Non-fatal
		//
		rts_warning(eh, "Accept of incoming connection produced invalid socket. WSAGetLastError is %d", err);

		new_client->value = -1;
		return false;
	} else {
		new_client->value = client;
		return true;
	}
}

bool socket_recv(rts_eh_t* eh, rts_sock_t sock, char* buffer, int buffer_length, int* bytes_read) {

	int result = recv(sock.value, buffer, buffer_length, 0); // TODO: PEEK should be separate

	if (result == SOCKET_ERROR) {		
		*bytes_read = -1;

		int err = WSAGetLastError();
		rts_warning(eh, "Receive from socket failed. WSAGetLastError is %d", err);

		return false;
	} else {

		*bytes_read = result;
		return true;
	}
}

bool socket_send(rts_eh_t* eh, rts_sock_t sock, char* buffer, int buffer_length, int* bytes_sent) {

	int result = send(sock.value, buffer, buffer_length, 0);

	if (result == SOCKET_ERROR) {
		*bytes_sent = -1;

		int err = WSAGetLastError();
		rts_warning(eh, "Send to socket peer failed. WSAGetLastError is %d", err);

		return false;
	} else {
		*bytes_sent = result;
		return true;
	}
}

bool socket_select(rts_eh_t* eh, rts_sock_set_t* recv, rts_sock_set_t* send) {

	fd_set* recv_fd = NULL;
	fd_set* send_fd = NULL;

	if (recv != NULL) {
		recv_fd = &((rts_sock_windows_sock_set_t*)recv->os_specific)->set;
	}

	if (send != NULL) {
		send_fd = &((rts_sock_windows_sock_set_t*)send->os_specific)->set;
	}
	
	int result = select(0, recv_fd, send_fd, NULL, NULL);

	if (result == SOCKET_ERROR) {

		rts_panic_winsock_error(eh, "Failed to select");

		return false;
	} else {
		return true;
	}
}

void add_sock_set(void* os_specific, rts_sock_t sock) {
	rts_sock_windows_sock_set_t* windows = (rts_sock_windows_sock_set_t*)os_specific;
	FD_SET(sock.value, &(windows->set));
}

void clear_sock_set(void* os_specific, rts_sock_t sock) {
	rts_sock_windows_sock_set_t* windows = (rts_sock_windows_sock_set_t*)os_specific;
	FD_CLR(sock.value, &(windows->set));
}

bool is_set_sock_set(void* os_specific, rts_sock_t sock) {
	rts_sock_windows_sock_set_t* windows = (rts_sock_windows_sock_set_t*)os_specific;
	return FD_ISSET(sock.value, &(windows->set)) != 0;
}

void destroy_sock_set(void* os_specific) {
	rts_sock_windows_sock_set_t* windows = (rts_sock_windows_sock_set_t*)os_specific;
	rts_free(windows);
}

rts_sock_set_t* create_sock_set() {
	rts_sock_set_t* set = rts_sock_create_set();
	set->destroy = &destroy_sock_set;
	set->add = &add_sock_set;
	set->clear = &clear_sock_set;
	set->is_set = &is_set_sock_set;

	rts_sock_windows_sock_set_t* windows = (rts_sock_windows_sock_set_t*)rts_alloc(0, sizeof(rts_sock_windows_sock_set_t));
	set->os_specific = windows;
	set->os_specific_size = sizeof(rts_sock_windows_sock_set_t);

	FD_ZERO(&(windows->set));

	return set;
}

void rts_sock_windows_attach(rts_sock_os_t* os) {
	os->global_start = &winsock_start;
	os->global_stop = &winsock_stop;

	os->open = &socket_open;
	os->close = &socket_close;

	os->listen = &socket_listen;
	os->bind = &socket_bind;
	os->accept = &socket_accept;
	os->recv = &socket_recv;
	os->send = &socket_send;

	os->select = &socket_select;
	os->create_sock_set = &create_sock_set;
}


#endif