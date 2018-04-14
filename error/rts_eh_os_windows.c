#include "rts_eh_os.h"

#ifdef RTS_PLAT_WINDOWS


void rts_panic_last_error(rts_eh_t* handler, const char* format) {

	DWORD last_error = GetLastError();

	rts_info(handler, "GetLastError returned %d", last_error);
	rts_panic(handler, format);
}

#endif