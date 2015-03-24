
#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "stdout.h"
#include "uart.h"

#include "platform_config.h"
#include "jsinteractive.h"
#include "jshardware.h"

#include "jsvar.h"
#include "jswrap_functions.h"
#include "spi_flash.h"


// error handler for pure virtual calls
void __cxa_pure_virtual() { while (1); }

void jsInit() {
    jshInit();
    jsvInit();
    jsiInit(true);
}

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
	static int state = 0;
	jsEval(state++);
	jsiLoop();
}
void runTimer() {
	static ETSTimer timer;
	os_timer_setfn(&timer, onTimer, NULL);
	os_timer_arm(&timer, 1000, true);
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
			os_printf("\nwriteToFlash error %d\n", error);
		}
		addr += SPI_FLASH_SEC_SIZE;
		code += SPI_FLASH_SEC_SIZE;
		sector++;
	}
}

void ICACHE_RAM_ATTR user_init(void) {
	uart_init(BIT_RATE_115200, 0);

	jsInit();

	os_printf("\nReady\n");
	
//	runTimer();

	os_printf("\nRead from flash:\n");
	char c;
	int error;
	int addr;
	JsVar *jsCode = jsvNewFromEmptyString();
	for (addr = 0x60000;; addr++) {
		if (SPI_FLASH_RESULT_OK != (error = spi_flash_read(addr, (uint32 *)&c, 1))) {
			os_printf("\nerror %d\n", error);
			jsvUnLock(jsCode);
			jsCode = NULL;
			break;
		}
		if (0x80 & c || 0 == c) break; // allow ascii only
		jsvAppendStringBuf(jsCode, &c, 1);
		uart0_putc(c);
	}
	if (jsCode && 0x60000 < addr) {
//		os_printf("\n%s\n", jsVarToString(jsCode));
		JsVar *jsResult = jswrap_eval(jsCode);
		jsvUnLock(jsCode); jsCode = NULL;
		os_printf("\nResult: %s\n", jsVarToString(jsResult));
		jsvUnLock(jsResult); jsResult = NULL;
	}
	
	bool cr = false;
	while (true) {
		while ((c = uart_getc())) {
			uart0_putc(c);
			if (cr && '\n' == c) {
				if (jsCode) {
					writeToFlash(jsCode);
					JsVar *jsResult = jswrap_eval(jsCode);
					jsvUnLock(jsCode); jsCode = NULL;
					os_printf("\n%s\n", jsVarToString(jsResult));
					jsvUnLock(jsResult); jsResult = NULL;
				}
			} else if (0 < c && !(cr = '\r' == c)) {
				if (!jsCode) jsCode = jsvNewFromEmptyString();
				jsvAppendStringBuf(jsCode, &c, 1);
			}
		}
		jsiLoop();
	}
}
