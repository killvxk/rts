#include "error/rts_eh.h"
#include <stdio.h>

int main()
{		
	rts_eh_t t = rts_eh_create_generic();
	rts_eh_t* log = &t;

	rts_info(log, "Test");

	RTS_ASSERT(NULL, 1 == 2);

	rts_panic(&t, "Hello world %d!", 42);

	rts_info(&t, "Hello again!");

	return 0;
}
