#pragma once
#include "rts_os.h"

#ifdef RTS_PLAT_WINDOWS

// Universal include of windows.h. Yawn.
//
#undef _WINSOCKAPI_
#define _WINSOCKAPI_

#include <windows.h>

#endif