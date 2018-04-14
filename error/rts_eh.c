#include "rts_eh.h"
#include <stdio.h>

typedef enum {

	RTS_EH_INFO,
	RTS_EH_PANIC

} rts_eh_internal_level;

void rts_eh_run(rts_eh_t* handler, rts_eh_internal_level level, const char* format, va_list args) {

	if (handler == NULL) {
		rts_eh_t fallback = rts_eh_create_generic();
		printf("!! NO HANDLER PROVIDED, CREATING AND USING STDOUT FALLBACK !!\n");
		handler = &fallback;
	}

	switch (level)
	{
		case RTS_EH_INFO: handler->info(format, args); break;

		default:
		case RTS_EH_PANIC: {
			handler->panic(format, args);
		}
		break;
	}
}

void rts_panic(rts_eh_t* handler, const char* format, ...) {

	va_list args;
	va_start(args, format);

	rts_eh_run(handler, RTS_EH_PANIC, format, args);

	va_end(args);
}

void rts_info(rts_eh_t* handler, const char* format, ...) {

	va_list args;
	va_start(args, format);

	rts_eh_run(handler, RTS_EH_INFO, format, args);

	va_end(args);
}
