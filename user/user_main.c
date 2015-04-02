
#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "stdout.h"
#include "uart.h"
#include "user_interface.h"

#include "platform_config.h"
#include "jsinteractive.h"
#include "jshardware.h"

#include "jsvar.h"
#include "jswrap_functions.h"
#include "spi_flash.h"


// error handler for pure virtual calls
void __cxa_pure_virtual() { while (1); }

void jsInit(bool autoLoad) {
	jshInit();
	jsvInit();
	jsiInit(autoLoad);
}

void jsKill() {
	jsiKill();
	jsvKill();
	jshKill();
}

/*
 void vApplicationMallocFailedHook( void );
 void *pvPortMalloc( size_t xWantedSize );
 void vPortFree( void *pv );
 size_t xPortGetFreeHeapSize( void );
 void vPortInitialiseBlocks( void );
*/
/*void *alloca(size_t s) {
	void *p = os_malloc(s);
	os_printf("alloca %p, %d\n", p, s);
	return p;
}*/

#define malloc os_malloc
#define free os_free
//#define realloc os_realloc
/*void *malloc(size_t s) {
	void *p = os_malloc(s);
	os_printf("malloc %p, %d\n", p, s);
	return p;
}
void free(void *p) {
	os_printf("free %p, %d\n", p, sizeof(p));
	os_free(p);
}
void *os_realloc(void *old, size_t size) {
	size_t s = sizeof(old);
	if (size <= s) return old;
	void *new = os_malloc(size);
	//os_printf("realloc %p, %d, %p, %d\n", old, s, new, size);
	memcpy(new, old, s < size ? s : size);
	os_free(old);
	return new;
}*/

const char *ICACHE_RAM_ATTR jsVarToString(JsVar *jsVar) {
	if (!jsVar) return "undefined";
	jsVar = jsvAsString(jsVar, true/*unlock*/);
	size_t len = jsvGetStringLength(jsVar);
	if (0 == len) return "''";
	static char *str = NULL;
	static size_t size = 0;
	if (!str) str = malloc(size = len+32);
	else if (size < len+1) {
		free(str);
		str = malloc(size = len+32);
	}
	len = jsvGetString(jsVar, str, size);
	str[len] = 0;
	return str ? str : "undefined";
}

extern UartDevice UartDev;

/*void ICACHE_RAM_ATTR onTimer(void *arg) {
//	static int state = 0;
//	jsEval(state++);
//	jsiLoop();
	os_printf("Heap size: %d\n", system_get_free_heap_size());
}
void runTimer() {
	static ETSTimer timer;
	os_timer_setfn(&timer, onTimer, NULL);
	os_timer_arm(&timer, 300, true);
}*/

void writeToFlash(JsVar *jsCode) {
	if (!jsCode) return;
	const char *code = jsVarToString(jsCode);
	int error;
	int addr = 0x60000;
	int sector = addr/SPI_FLASH_SEC_SIZE;
	int to = addr + strlen(code)+1;
	while (addr < to) {
		spi_flash_erase_sector(sector);
		if (SPI_FLASH_RESULT_OK != (error = spi_flash_write(addr, (uint32 *)code, SPI_FLASH_SEC_SIZE))) {
			jsiConsolePrintf("\nwriteToFlash error %d\n", error);
		}
		addr += SPI_FLASH_SEC_SIZE;
		code += SPI_FLASH_SEC_SIZE;
		sector++;
	}
}

void printTime(JsSysTime time) {
	JsVarFloat ms = jshGetMillisecondsFromTime(time);
	jsiConsolePrintf("time: %d, %f, %d\n", (int)time, ms, (int)jshGetTimeFromMilliseconds(ms));
}
void printMilliseconds(JsVarFloat ms) {
	JsSysTime time = jshGetTimeFromMilliseconds(ms);
	jsiConsolePrintf("ms: %f, %d, %f\n", ms, (int)time, jshGetMillisecondsFromTime(time));
}

void test() {
	JsSysTime time = 1;
	for (int n = 0; n < 15; n++) {
		printTime(time);
		printTime(time/10);
		time *= 10;
	}
	JsVarFloat ms = 1.0;
	for (int n = 0; n < 15; n++) {
		printMilliseconds(ms);
		printMilliseconds(ms/10.0);
		ms *= 10.0;
	}
}

//JsVar *jswrap_interface_setInterval(JsVar *func, JsVarFloat timeout);
JsVar *prototype1(JsVar *v1, JsVarFloat f1) {
	jsiConsolePrintf("%v, %d\n", v1, (int)f1);
	return 0;
}
JsVar *prototype2(JsVar *v1, JsVar *v2, JsVarFloat f1, JsVarFloat f2) {
	jsiConsolePrintf("%v, %v, %d, %d\n", v1, v2, (int)f1, (int)f2);
	return 0;
}
JsVar *prototype3(JsVar *v1, JsVar *v2, JsVar *v3, JsVarFloat f1, JsVarFloat f2, JsVarFloat f3) {
	jsiConsolePrintf("%v, %v, %v, %d, %d, %d\n", v1, v2, v3, (int)f1, (int)f2, (int)f3);
	return 0;
}
JsVar *prototype4(JsVar *v1, JsVar *v2, JsVar *v3, JsVar *v4, JsVarFloat f1, JsVarFloat f2, JsVarFloat f3, JsVarFloat f4) {
	jsiConsolePrintf("%v, %v, %v, %v, %d, %d, %d, %d\n", v1, v2, v3, v4, (int)f1, (int)f2, (int)f3, (int)f4);
	return 0;
}

void functionCall4(void *function, JsVar *v1, JsVar *v2, JsVar *v3, JsVar *v4, JsVar *v5, JsVar *v6, JsVar *v7, JsVar *v8) {
	size_t d[] = {(size_t)v1, (size_t)v2, (size_t)v3, (size_t)v4};
	JsVarFloat f[] = {jsvGetFloat(v5), jsvGetFloat(v6), jsvGetFloat(v7), jsvGetFloat(v8)};
	
	((uint64_t (*) (
					size_t,
					size_t,
					JsVarFloat,
					JsVarFloat
					))function) (
								 d[0],
								 d[1],
								 f[0],
								 f[1]
								 );
}

void functionCall8(void *function, JsVar *v1, JsVar *v2, JsVar *v3, JsVar *v4, JsVar *v5, JsVar *v6, JsVar *v7, JsVar *v8) {
	size_t d[] = {(size_t)v1, (size_t)v2, (size_t)v3, (size_t)v4};
	JsVarFloat f[] = {jsvGetFloat(v5), jsvGetFloat(v6), jsvGetFloat(v7), jsvGetFloat(v8)};
	
	((uint64_t (*) (
					size_t,
					size_t,
					size_t,
					size_t,
					JsVarFloat,
					JsVarFloat,
					JsVarFloat,
					JsVarFloat
					))function) (
								 d[0],
								 d[1],
								 d[2],
								 d[3],
								 f[0],
								 f[1],
								 f[2],
								 f[3]
								 );
}

void testFunctionCall() {
	JsVar *v[] = {
		jsvNewFromFloat(11.0), jsvNewFromFloat(12.0), jsvNewFromFloat(13.0), jsvNewFromFloat(14.0),
		jsvNewFromFloat(15.0), jsvNewFromFloat(16.0), jsvNewFromFloat(17.0), jsvNewFromFloat(18.0)
	};
	
	functionCall4(prototype1, v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
	functionCall4(prototype2, v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
	functionCall8(prototype3, v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
	functionCall8(prototype4, v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
}

#include "jswrapper.h"
void addNativeFunction(const char *name, void (*callbackPtr)(void)) {
	jsvUnLock(jsvObjectSetChild(execInfo.root, name, jsvNewNativeFunction(callbackPtr, JSWAT_VOID)));
}

void nativeSave() {
	jsiConsolePrintf("nativeSave\n");
}

void ICACHE_RAM_ATTR user_init(void) {
	uart_init(BIT_RATE_115200, 0);
	os_printf("Heap size: %d\n", system_get_free_heap_size());

	jsInit(true);
	addNativeFunction("save", nativeSave);
	addNativeFunction("discard", nativeDiscard);

	//testFunctionCall();
	system_print_meminfo();

	jsiConsolePrintf("\nReady\n");
	os_printf("Heap size: %d\n", system_get_free_heap_size());

	//runTimer();

	jsiConsolePrintf("\nRead from flash:\n");
	char c;
	int error;
	int addr;
	JsVar *jsCode = jsvNewFromEmptyString();
	for (addr = 0x60000;; addr++) {
		if (SPI_FLASH_RESULT_OK != (error = spi_flash_read(addr, (uint32 *)&c, 1))) {
			jsiConsolePrintf("\nerror %d\n", error);
			jsvUnLock(jsCode);
			jsCode = 0;
			break;
		}
		if (0x80 & c || 0 == c) break; // allow ascii only
		jsvAppendStringBuf(jsCode, &c, 1);
		uart0_putc(c);
	}
	if (jsCode && 0x60000 < addr) {
	}
	else {
		char *code = " \
var v = 0, g = new Pin(12), b = new Pin(13), r = new Pin(15); \
\
setInterval(function() { \
  r.write(v&1); \
  g.write(v>>1&1); \
  b.write(v>>2&1); \
  console.log('timer: ', v++); \
}, 100); \
save(); \
";
		jsiConsolePrintf(code);
		jsCode = jsvNewFromString(code);
	}
	if (jsCode) {
		JsVar *jsResult = jspEvaluateVar(jsCode, 0, true);
		jsvUnLock(jsCode); jsCode = 0;
		if (jsResult) {
			jsiConsolePrintf("%v\n", jsResult);
			jsvUnLock(jsResult);
		}
	}
	os_printf("Heap size: %d\n", system_get_free_heap_size());

	bool cr = false;
	while (true) {
		while ((c = uart_getc())) {
			uart0_putc(c);
			if (cr && '\n' == c) {
				if (jsCode) {
//					writeToFlash(jsCode);
					JsVar *jsResult = jspEvaluateVar(jsCode, 0, true);
					jsvUnLock(jsCode); jsCode = 0;
					jsiConsolePrintf("%v\n", jsResult);
					jsvUnLock(jsResult);
				}
			} else if (0 < c && !(cr = '\r' == c)) {
				if (!jsCode) jsCode = jsvNewFromEmptyString();
				jsvAppendStringBuf(jsCode, &c, 1);
			}
		}
		os_printf("Heap size: %d\n", system_get_free_heap_size());
		jsiLoop();
	}
	jsKill();
}

