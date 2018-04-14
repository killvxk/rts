#include "rts_eh_os.h"

#ifdef RTS_PLAT_LINUX
#include <errno.h>

void rts_panic_unix_errno(rts_eh_t* handler, const char* format) {

	int current_errno = errno;

	rts_info(handler, "errno is %d", current_errno);
	rts_panic(handler, format);
}


#endif