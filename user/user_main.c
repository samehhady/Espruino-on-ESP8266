
#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "stdout.h"
#include "uart.h"

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
 void vApplicationMallocFailedHook( void );
 void *pvPortMalloc( size_t xWantedSize );
 void vPortFree( void *pv );
 size_t xPortGetFreeHeapSize( void );
 void vPortInitialiseBlocks( void );
*/
#define malloc os_malloc
#define free os_free
#define realloc os_realloc
void *ICACHE_RAM_ATTR os_realloc(void *old, size_t size) {
	size_t s = sizeof(old);
	if (size <= s) return old;
	void *new = os_malloc(size);
	memcpy(new, old, s < size ? s : size);
	os_free(old);
	return new;
}

const char *ICACHE_RAM_ATTR jsVarToString(JsVar *jsVar) {
	if (!jsVar) return "undefined";
	jsVar = jsvAsString(jsVar, true/*unlock*/);
	size_t len = jsvGetStringLength(jsVar);
	if (0 == len) return "''";
	static char *str = NULL;
	static size_t size = 0;
	if (!str) str = malloc(size = len+32);
	else if (size < len) {
		free(str);
		str = malloc(size = len+32);
	}
	len = jsvGetString(jsVar, str, size);
	str[len] = 0;
	return str ? str : "undefined";
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
void ICACHE_RAM_ATTR jsEval(int n) {
	if (5 < n) return;
	os_printf("\n----- jsEval %d: -----\n\n", n);
	JsVar *jsCode = jsvNewFromString(code[n]); os_printf("jsCode:\n\n%s\n\n", jsVarToString(jsCode));
	JsVar *jsResult = jswrap_eval(jsCode); os_printf("jsResult:\n\n%s\n\n", jsVarToString(jsResult));
}

extern UartDevice UartDev;
//extern int uartRecvCounter;

void ICACHE_RAM_ATTR onTimer(void *arg) {
	static int state = 0;
	jsEval(state++);
	jsiLoop();

/*	char c = uart_getc();

	os_printf("%d, %d, %d, %p, %p, %p, %c\n\r",
			  UartDev.baut_rate,
			  UartDev.rcv_buff.RcvBuffSize,
			  UartDev.rcv_buff.BuffState,
			  UartDev.rcv_buff.pRcvMsgBuff,
			  UartDev.rcv_buff.pWritePos,
			  UartDev.rcv_buff.pReadPos,
			  c
//			  uartRecvCounter
	);*/
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

void ICACHE_RAM_ATTR user_init(void) {
//	ets_delay_us(1000);
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
//	ets_delay_us(1000);

	jsInit();
	jsVar();

//	os_printf("user_init\n");
//	uart0_sendStr("uart0_sendStr\n");
	
//    stdoutInit();
	os_printf("Ready\n");
	
	runTimer();
//	static JsVar *jsReceive = NULL;
//	jsReceive =
	//jsvNewFromString("");
/*	jsEval(0);
	jsEval(1);
	int n = 0;
	os_printf("\n----- jsEval %d: -----\n\n", n);
	JsVar *jsCode = jsvNewFromString(code[n]); os_printf("jsCode:\n\n%s\n\n", jsVarToString(jsCode));
	JsVar *jsResult = jswrap_eval(jsCode); os_printf("jsResult:\n\n%s\n\n", jsVarToString(jsResult));
*/
	/*
  jsvUnLock(jspEvaluate(buffer, true));
	 
  isRunning = true;
  bool isBusy = true;
  while (isRunning && (jsiHasTimers() || isBusy))
	 isBusy = jsiLoop();
	 
  JsVar *result = jsvObjectGetChild(execInfo.root, "result", 0);//no create
	bool pass = jsvGetBool(result);
	jsvUnLock(result);
	
	 */
	
	
	while (true) {
		char c;
		static bool cr = false;
		static JsVar *jsReceive = NULL;
		while ((c = uart_getc())) {
			uart0_putc(c);
			if (cr && '\n' == c) {
//				jswrap_interface_print(jsReceive);
//				os_printf("code:\n\n%s\n\n", jsVarToString(jsReceive));
				JsVar *jsResult = jswrap_eval(jsReceive);
				os_printf("%s\n\n", jsVarToString(jsResult));
//				jswrap_interface_print(jsResult);
//				JsVar *jsResult = jspEvaluate(jsReceive);
//				os_printf("result:\n\n%s\n\n", jsVarToString(jsResult));
//				jsvUnLock(jsResult);
//				jsvUnLock(jsReceive);
				jsReceive = NULL;
			} else if (!(cr = '\r' == c)) {
				if (!jsReceive) jsReceive = jsvNewFromEmptyString();
//				if (!jsReceive) jsReceive = jsvNewWithFlags(JSV_STRING_0);
//				if (!jsReceive) jsReceive = jsvNewFromString("");
				jsvAppendStringBuf(jsReceive, &c, 1);
			}
		}
		jsiLoop();
	}
//	ioInit();
	
//    jsMain();
}

