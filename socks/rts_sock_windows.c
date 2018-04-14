#include "rts_os.h"

#ifdef  RTS_PLAT_WINDOWS
#include "rts_sock_windows.h"

void rts_sock_windows_attach(rts_sock_os_t* sock) {
	sock->os_start = &rts_sock_windows_start;
	sock->os_stop = &rts_sock_windows_stop;
}

bool rts_sock_windows_start(rts_eh_t* eh) {
	WSADATA wsa_data;

	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) == 0) {

		rts_info(eh, "Winsock started OK");

		return true;

	} else {
			
		rts_panic_last_error(eh, "Winsock failed to start");

		return false;
	}
}

void rts_sock_windows_stop(rts_eh_t* eh) {
	if (WSACleanup() == 0) {
		rts_info(eh, "Winsock stopped OK");
	} else {
		rts_panic_last_error(eh, "Winsock failed to cleanup");
	}
}


#endif