
#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "stdout.h"

#include "platform_config.h"
#include "jsinteractive.h"
#include "jshardware.h"

// error handler for pure virtual calls
void __cxa_pure_virtual() { while (1); }

int jsMain() {
	jshInit();
	jsvInit();
	jsiInit(true);
	
	while (1)
		jsiLoop();
	
	// js*Kill()
}

void user_init(void) {
	stdoutInit();
//	ioInit();
	
	jsMain();
}

