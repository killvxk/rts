#include "rts_sock_os.h"
#include "rts_os.h"
#include "rts_sock_windows.h"

static bool dummy_start(rts_eh_t* eh) {
	return true;
}

static void dummy_stop(rts_eh_t* eh) {
}

rts_sock_os_t rts_sock_os_create(rts_eh_t* eh) {

	rts_sock_os_t sock;
	sock.os_start = &dummy_start;
	sock.os_stop = &dummy_stop;

#ifdef RTS_PLAT_WINDOWS

	// Attach windows starters/stoppers if appropriate
	rts_sock_windows_attach(&sock);

#endif
	
	return sock;
}