#pragma once
#include "rts_os.h"
#include "rts_eh.h"

// OS-specific error handler/helper logic
#ifdef RTS_PLAT_WINDOWS

#include <windows.h>

// Logs the last Win32 error and then proceeds to error
void rts_panic_last_error(rts_eh_t* handler, const char* format);

// Logs the last winsock error and then proceeds to error
void rts_panic_winsock_error(rts_eh_t* handler, const char* format);


#endif

#ifdef RTS_PLAT_LINUX

// Logs the last errno value and then proceeds to error
void rts_panic_unix_errno(rts_eh_t* handler, const char* format);

#endif