#include "rts_eh_os.h"

#ifdef RTS_PLAT_WINDOWS

#include "rts_windows.h"
#include <winsock2.h>

void rts_panic_last_error(rts_eh_t* handler, const char* format) {

	DWORD last_error = GetLastError();

	rts_info(handler, "GetLastError returned %d", last_error);
	rts_panic(handler, format);
}

void rts_panic_winsock_error(rts_eh_t* handler, const char* format) {

	// Codes: https://msdn.microsoft.com/en-us/library/windows/desktop/ms740668(v=vs.85).aspx
	DWORD last_error = WSAGetLastError();

	rts_info(handler, "WSAGetLastError returned %d", last_error);
	rts_panic(handler, format);
}

#endif