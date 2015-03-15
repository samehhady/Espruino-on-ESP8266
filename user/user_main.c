
#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "stdout.h"

// JS begin
#include "platform_config.h"
#include "jsinteractive.h"
#include "jshardware.h"

// error handler for pure virtual calls
void __cxa_pure_virtual() { while (1); }

void jsMain() {
	os_printf("jshInit\n");
    jshInit();
	os_printf("jsvInit\n");
    jsvInit();
	os_printf("jsiInit\n");
    jsiInit(true);

//    while (1) {
//        jsiLoop();
	
    // js*Kill()
}
// JS end

static ETSTimer timer;
static bool init = true;

void onTimer(void *arg) {
	if (init) {
		init = false;
		jsMain();
	}
	else {
		os_printf("jsiLoop\n");
		jsiLoop();
	}
	os_printf("alive!\n");
}

void runTimer() {
	os_timer_setfn(&timer, onTimer, NULL);
	os_timer_arm(&timer, 500, true);
}


void user_init(void) {
	os_printf("user_init\n");
    stdoutInit();
	os_printf("Ready\n");
	
	runTimer();
//	ioInit();
	
//    jsMain();
}

