#pragma once
#include <stdarg.h>

typedef void rts_error_handler(const char* format, va_list args);

// Error and logging handler
//
// Intention is that different handler instances can be passed into different
// methods at diff. levels in the stack (e.g. specific instance for logging 
// networking errors/messages etc.)
//
typedef struct {

	rts_error_handler* panic;

	rts_error_handler* info;

} rts_eh_t;

// Create a generic error and logging handler 
rts_eh_t rts_eh_create_generic();

void rts_info(rts_eh_t* handler, const char* format, ...);

void rts_panic(rts_eh_t* handler, const char* format, ...);

// Assertion that always panics (not debug only)
#define RTS_ASSERT(handler_ptr, condition) \
	if (!(condition)) { rts_panic(handler_ptr, "Assertion failed: %s (%s:%d)", #condition, __FILE__, __LINE__); }

