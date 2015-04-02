
// Automatically generated header file for ESP8266
// Generated by scripts/build_platform_config.py

#ifndef _PLATFORM_CONFIG_H
#define _PLATFORM_CONFIG_H


#define PC_BOARD_ID "ESP8266"
#define PC_BOARD_CHIP "ESP8266"
#define PC_BOARD_CHIP_FAMILY "ESP8266"
#define EMBEDDED

//#define __WORDSIZE 32
//#define FAKE_STDLIB

//#define USE_FLOATS

#include <stdarg.h>

#include "espmissingincludes.h"
//#include "c_types.h"
//#include "user_interface.h"
//#include "espconn.h"
#include "mem.h"
#include "alloca.h"
#include "osapi.h"

//typedef sint64_t int64_t;
//#define int64_t sint64_t

//#define itostr(a, b, c) os_sprintf(b, 16 == c ? "%x" : "%d", a)
//#define alloca os_malloc
//void *alloca(size_t size);
extern int isnan ( double );
extern int isfinite ( double );
extern double INFINITY, NAN;
#define RAND_MAX (0xFFFFFFFFU)

/*void __exit(int n) {
	os_printf("EXIT %d", n);
}*/
#define exit(n) jsiConsolePrintf("EXIT %d", n)
//void vcbprintf(vcbprintf_callback user_callback, void *user_data, const char *fmt, va_list argp);


// SYSTICK is the counter that counts up and that we use as the real-time clock
// The smaller this is, the longer we spend in interrupts, but also the more we can sleep!
#define SYSTICK_RANGE 0x1000000 // the Maximum (it is a 24 bit counter) - on Olimexino this is about 0.6 sec
#define SYSTICKS_BEFORE_USB_DISCONNECT 2

#define DEFAULT_BUSY_PIN_INDICATOR (Pin)-1 // no indicator
#define DEFAULT_SLEEP_PIN_INDICATOR (Pin)-1 // no indicator

// When to send the message that the IO buffer is getting full
#define IOBUFFER_XOFF ((TXBUFFERMASK)*6/8)
// When to send the message that we can start receiving again
#define IOBUFFER_XON ((TXBUFFERMASK)*3/8)

/*
 data  : 0x3ffe8000 ~ 0x3ffe8a54, len: 2644
 rodata: 0x3ffe8a60 ~ 0x3ffed78c, len: 19756
 bss   : 0x3ffed790 ~ 0x3fff89a8, len: 45592
 heap  : 0x3fff89a8 ~ 0x3fffc000, len: 13912
*/
//#define RESIZABLE_JSVARS
#define RAM_TOTAL (13*1024)//13640//(32*1024)
#define FLASH_TOTAL 0x80000//(512*1024)

#define JSVAR_CACHE_SIZE                200//1023 // Number of JavaScript variables in RAM
#define FLASH_AVAILABLE_FOR_CODE        0x20000
#define FLASH_PAGE_SIZE                 0x200//512
#define FLASH_SAVED_CODE_PAGES          0x100//16
#define FLASH_START                     0x60000

#define FLASH_SAVED_CODE_LENGTH (FLASH_PAGE_SIZE*FLASH_SAVED_CODE_PAGES)
#define FLASH_SAVED_CODE_START (FLASH_START + FLASH_TOTAL - FLASH_SAVED_CODE_LENGTH)
#define FLASH_MAGIC_LOCATION (FLASH_SAVED_CODE_START + FLASH_SAVED_CODE_LENGTH - 4)
#define FLASH_MAGIC 0xDEADBEEF

#define USARTS                          1
#define SPIS                            1
#define I2CS                            1
#define ADCS                            1
#define DACS                            1

#define DEFAULT_CONSOLE_DEVICE              EV_SERIAL1

#define IOBUFFERMASK 127 // (max 255) amount of items in event buffer - events take ~9 bytes each
#define TXBUFFERMASK 127 // (max 255)
#define UTILTIMERTASK_TASKS (16) // Must be power of 2 - and max 256


// definition to avoid compilation when Pin/platform config is not defined
#define IS_PIN_USED_INTERNALLY(PIN) ((false))
#define IS_PIN_A_LED(PIN) ((false))
#define IS_PIN_A_BUTTON(PIN) ((false))

#endif // _PLATFORM_CONFIG_H
