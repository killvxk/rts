#pragma once
#include "rts_os.h"
#include "rts_eh.h"

// OS-specific error handler/helper logic
#ifdef RTS_PLAT_WINDOWS

#include <windows.h>

// Logs the last Win32 error and then proceeds to error
void rts_panic_last_error(rts_eh_t* handler, const char* format);

#endif