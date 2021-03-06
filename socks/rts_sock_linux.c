#include "rts_sock_linux.h"

#ifdef RTS_PLAT_LINUX

#include "rts_alloc.h"
#include <string.h>

rts_sock_t socket_open(rts_eh_t* eh) {

	rts_sock_t sock;
	sock.value = socket(AF_INET, SOCK_STREAM, 0);

	if (sock.value < 0) {
		rts_panic_unix_errno(eh, "Open generated invalid socket");
	}

	int enable = 1;

	// Always permit reuse of address
	if (setsockopt(sock.value, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(enable)) != 0) {
		rts_panic_unix_errno(eh, "Failed to enable address reuse for new socket");
	}

	// Do not await buffered writes before shutdown
	//
	struct linger linger_opt; 
	linger_opt.l_onoff = 0;
	linger_opt.l_linger = 0;

	if (setsockopt(sock.value, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(struct linger)) != 0) {
		rts_panic_unix_errno(eh, "Failed to enable non-linger for new socket");
	}

	// Do not use Nagle's algorithm
	if (setsockopt(sock.value, IPPROTO_TCP, TCP_NODELAY, (const char*)&enable, sizeof(enable)) != 0) {
		rts_panic_unix_errno(eh, "Failed to disable Nagle's algorithm for new socket");
	}

	// Enable non-blocking mode
	if (fcntl(sock.value, F_SETFL, O_NONBLOCK) != 0) {
		rts_panic_unix_errno(eh, "Failed to enable non-blocking for new socket");
	}

	return sock;
}

void socket_close(rts_eh_t* eh, rts_sock_t sock) {
	
	if (close(sock.value) != 0) {
		rts_panic_unix_errno(eh, "Failed to close socket!");
	}
}

bool socket_bind(rts_eh_t* eh, rts_sock_t sock, int port) {
	struct addrinfo hint_struct;
	memset(&hint_struct, 0, sizeof(struct addrinfo));

	hint_struct.ai_family = AF_INET;
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

	if (bind(sock.value, result->ai_addr, result->ai_addrlen) == 0) {
		freeaddrinfo(result);
		rts_info(eh, "Bound socket %d to port %d", sock.value, port);
		return true;
	} else {
		// Sets errno on failure
		rts_panic_unix_errno(eh, "Failed to bind socket to port");
		freeaddrinfo(result);
		return false;
	}
}

bool socket_listen(rts_eh_t* eh, rts_sock_t sock) {

	if (listen(sock.value, SOMAXCONN) == 0) {
		rts_info(eh, "Listening started on socket %d", sock.value);
		return true;
	} else {
		// Sets errno on failure
		rts_panic_unix_errno(eh, "Failed to start listening on socket");
		return false;
	}
}

bool socket_accept(rts_eh_t* eh, rts_sock_t listener, rts_sock_t* new_client) {

	int client = accept(listener.value, NULL, NULL);

	if (client < 0) {

		int current_errno = errno;

		rts_warning(eh, "Accept of incoming connection produced invalid socket. errno is %d", current_errno);

		new_client->value = -1;
		return false;
	} else {
		new_client->value = client;		
		return true;
	}
}

bool socket_recv(rts_eh_t* eh, rts_sock_t sock, char* buffer, int buffer_length, int* bytes_read, bool* would_block) {

	int result = recv(sock.value, buffer, buffer_length, 0); // TODO: PEEK should be separate

	if (result < 0) {
		*bytes_read = -1;

		int current_errno = errno;

		if (current_errno == EAGAIN || current_errno == EWOULDBLOCK) {
			// Blocking is not an error as such, but we do not succeed
			*would_block = true;
			return false;
		}

		rts_warning(eh, "Receive from socket failed. errno is %d", current_errno);

		return false;
	} else {
		*bytes_read = result;
		return true;
	}
}

bool socket_send(rts_eh_t* eh, rts_sock_t sock, char* buffer, int buffer_length, int* bytes_sent, bool* would_block) {
	int result = send(sock.value, buffer, buffer_length, 0);

	if (result < 0) {
		*bytes_sent = -1;

		int current_errno = errno;
		
		if (current_errno == EAGAIN || current_errno == EWOULDBLOCK) {
			// Blocking is not an error as such, but we do not succeed
			*would_block = true;
			return false;
		}

		rts_warning(eh, "Send to socket peer failed. errno is %d", current_errno);

		return false;
	} else {
		*bytes_sent = result;
		return true;
	}
}

bool socket_select(rts_eh_t* eh, rts_sock_set_t* recv, rts_sock_set_t* send) {

	fd_set* recv_fd = NULL;
	fd_set* send_fd = NULL;

	// We have to calculate highest file descriptor across EVERY set
	//
	int highest = -1;

	if (recv != NULL) {
		recv_fd = &((rts_sock_linux_sock_set_t*)recv->os_specific)->set;

		// Initial value for highest descriptor
		highest = ((rts_sock_linux_sock_set_t*)recv->os_specific)->highest;
	}

	if (send != NULL) {
		send_fd = &((rts_sock_linux_sock_set_t*)send->os_specific)->set;

		// Now max against the first value vs. the highest in this set
		highest = RTS_MAX(highest, ((rts_sock_linux_sock_set_t*)recv->os_specific)->highest);
	}

	int result = select(highest+1, recv_fd, send_fd, NULL, NULL);

	if (result < 0) {

		// Sets errno.		
		rts_panic_unix_errno(eh, "Failed to select");

		return false;
	}
	else {
		return true;
	}
}

void add_sock_set(void* os_specific, rts_sock_t sock) {
	rts_sock_linux_sock_set_t* lin = (rts_sock_linux_sock_set_t*)os_specific;
	FD_SET(sock.value, &(lin->set));

	lin->highest = RTS_MAX(lin->highest, sock.value);
}

void clear_sock_set(void* os_specific, rts_sock_t sock) {
	rts_sock_linux_sock_set_t* lin = (rts_sock_linux_sock_set_t*)os_specific;
	FD_CLR(sock.value, &(lin->set));

	// TODO: No good way to modify highest known descriptor value (ever useful in practice?)
}

bool is_set_sock_set(void* os_specific, rts_sock_t sock) {
	rts_sock_linux_sock_set_t* lin = (rts_sock_linux_sock_set_t*)os_specific;
	return FD_ISSET(sock.value, &(lin->set)) != 0;
}

void destroy_sock_set(void* os_specific) {
	rts_sock_linux_sock_set_t* lin = (rts_sock_linux_sock_set_t*)os_specific;
	lin->highest = -1;
	rts_free(lin);
}

rts_sock_set_t* create_sock_set() {
	rts_sock_set_t* set = rts_sock_create_set();
	set->destroy = &destroy_sock_set;
	set->add = &add_sock_set;
	set->clear = &clear_sock_set;
	set->is_set = &is_set_sock_set;

	rts_sock_linux_sock_set_t* lin = (rts_sock_linux_sock_set_t*)rts_alloc(0, sizeof(rts_sock_linux_sock_set_t));
	set->os_specific = lin;
	set->os_specific_size = sizeof(rts_sock_linux_sock_set_t);

	FD_ZERO(&(lin->set));
	lin->highest = -1;

	return set;
}

void rts_sock_linux_attach(rts_sock_os_t* os) {
	os->open = &socket_open;
	os->close = &socket_close;

	os->bind = &socket_bind;
	os->listen = &socket_listen;

	os->accept = &socket_accept;

	os->recv = &socket_recv;
	os->send = &socket_send;

	os->select = &socket_select;
	os->create_sock_set = &create_sock_set;
}

#endif