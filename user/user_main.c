
#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "stdout.h"
//#include "uart.h"

// JS begin
#include "platform_config.h"
#include "jsinteractive.h"
#include "jshardware.h"

#include "jsvar.h"
#include "jswrap_functions.h"

// error handler for pure virtual calls
void __cxa_pure_virtual() { while (1); }

void jsInit() {
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

/*
 void vApplicationMallocFailedHook( void ) ;//ICACHE_FLASH_ATTR;
 void *pvPortMalloc( size_t xWantedSize ) ;//ICACHE_FLASH_ATTR;
 void vPortFree( void *pv ) ;//ICACHE_FLASH_ATTR;
 size_t xPortGetFreeHeapSize( void ) ;//ICACHE_FLASH_ATTR;
 void vPortInitialiseBlocks( void ) ;//ICACHE_FLASH_ATTR;
*/
#define malloc os_malloc
#define free os_free
#define realloc os_realloc
inline void *os_realloc(void *old, size_t size) {
	void *new = os_malloc(size);
	size_t s = sizeof(old);
	memcpy(new, old, s < size ? s : size);
	os_free(old);
}

const char *jsVarToString(JsVar *jsVar) {
	if (!jsVar) return NULL;
	jsVar = jsvAsString(jsVar, true/*unlock*/);
	size_t len = jsvGetStringLength(jsVar);
	if (0 == len) return "";
	static char *str = NULL;
	static size_t size = 0;
	if (!str) str = malloc(size = len+32);
	else if (size < len) {
		free(str);
		str = malloc(size = len+32);
	}
	len = jsvGetString(jsVar, str, size);
	str[len] = 0;
	return str;
}
void jsVar() {
	JsVar *jsString = jsvNewFromString("It works!");
	
	os_printf("jsvGetTypeOf: %s\n", jsvGetTypeOf(jsString));
//	os_printf("jsvGetFlatStringPointer: %s\n", jsvGetFlatStringPointer(jsString));
	os_printf("jsvGetStringLength: %d\n", jsvGetStringLength(jsString));
	os_printf("jsvIsStringEqual (true): %d\n", jsvIsStringEqual(jsString, "It works!"));
	os_printf("jsvIsStringEqual (false): %d\n", jsvIsStringEqual(jsString, "It works again!"));

	os_printf("\n");
	os_printf("jsVarToString: %s\n", jsVarToString(jsString));
	os_printf("jsVarToString (1): %s\n", jsVarToString(jsvNewFromInteger(1)));
	os_printf("jsVarToString (0.1): %s\n", jsVarToString(jsvNewFromFloat(0.1)));
	os_printf("jsVarToString (true): %s\n", jsVarToString(jsvNewFromBool(true)));
}

static const char *code[] = {
	"'It works!'",
	"NULL || 'It works!'",
	"(function() {return 'It works!'; }())",
	"(function() {return 'It works!'; })()",
	"(function(a) {return a; })('It works!')",
	"console.log('It works!');"
};
void jsEval(int n) {
	if (5 < n) return;
	os_printf("\n----- jsEval %d: -----\n\n", n);
	JsVar *jsCode = jsvNewFromString(code[n]); os_printf("jsCode:\n\n%s\n\n", jsVarToString(jsCode));
	JsVar *jsResult = jswrap_eval(jsCode); os_printf("jsResult:\n\n%s\n\n", jsVarToString(jsResult));
}

void onTimer(void *arg) {
	static int state = 0;

	jsEval(state++);

	jsiLoop();
}

void runTimer() {
	static ETSTimer timer;
	os_timer_setfn(&timer, onTimer, NULL);
	os_timer_arm(&timer, 1000, true);
}
//void uart_init(UartBautRate uart0_br, UartBautRate uart1_br)
//void uart0_tx_buffer(uint8_t *buffer, uint16_t length);

//void uart0_rx_intr_handler(void *param) { // RcvMsgBuff *
	
//}


void user_init(void) {
	os_printf("user_init\n");
    stdoutInit();
//	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	os_printf("Ready\n");
	
	runTimer();
	
	jsInit();
	jsVar();
//	ioInit();
	
//    jsMain();
}

