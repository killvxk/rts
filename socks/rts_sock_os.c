#include "rts_sock_os.h"
#include "rts_os.h"
#include "rts_sock_windows.h"
#include "rts_sock_linux.h"

#include <stdio.h>
#include "rts_alloc.h"

static bool dummy_start(rts_eh_t* eh) {
	return true;
}

static void dummy_stop(rts_eh_t* eh) {
}

rts_sock_os_t rts_sock_os_create(rts_eh_t* eh) {

	rts_sock_os_t os;
	os.global_start = &dummy_start;
	os.global_stop = &dummy_stop;

#ifdef RTS_PLAT_WINDOWS
	rts_sock_windows_attach(&os);
#endif

#ifdef RTS_PLAT_LINUX
	rts_sock_linux_attach(&os);
#endif
	
	return os;
}

char* rts_sock_parse_port(int port) {

	// 65535 = max 5 chars + NULL TERMINATOR
	char* name = rts_alloc(0, sizeof(char) * 6);
	memset(name, 0, 6);
	snprintf(name, 6, "%d", port);

	return name;
}