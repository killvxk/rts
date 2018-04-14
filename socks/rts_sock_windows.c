#include "rts_os.h"

#ifdef  RTS_PLAT_WINDOWS
#include "rts_sock_windows.h"

bool winsock_start(rts_eh_t* eh) {
	WSADATA wsa_data;

	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) == 0) {
		rts_info(eh, "Winsock started OK");
		return true;
	} else {
		rts_panic_last_error(eh, "Winsock failed to start");
		return false;
	}
}

void winsock_stop(rts_eh_t* eh) {
	if (WSACleanup() == 0) {
		rts_info(eh, "Winsock stopped OK");
	} else {
		rts_panic_last_error(eh, "Winsock failed to cleanup");
	}
}

rts_sock_t socket_open(rts_eh_t* eh) {
	rts_sock_t sock;
	sock._value = socket(AF_INET, SOCK_STREAM, 0);

	return sock;
}

void socket_close(rts_eh_t* eh, rts_sock_t sock) {
	if (closesocket(sock._value) != 0) {
		rts_panic_last_error(eh, "Failed to close socket");
	}
}

void rts_sock_windows_attach(rts_sock_os_t* sock) {
	sock->global_start = &winsock_start;
	sock->global_stop = &winsock_stop;

	sock->open = &socket_open;
	sock->close = &socket_close;
}


#endif