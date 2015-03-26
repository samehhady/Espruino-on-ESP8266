/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Platform Specific part of Hardware interface Layer
 * ----------------------------------------------------------------------------
 */
//#include "Arduino.h"
//#include "HardwareSerial.h"

//#define CPLUSPLUS
//extern "C" {
#include "jshardware.h"
#include "jsutils.h"
#include "jsparse.h"
#include "jsinteractive.h"

#include "gpio.h"
#include "user_interface.h"

void jshUSARTInitInfo(JshUSARTInfo *inf) {
	inf->baudRate = DEFAULT_BAUD_RATE;
	inf->pinRX    = PIN_UNDEFINED;
	inf->pinTX    = PIN_UNDEFINED;
	inf->bytesize = DEFAULT_BYTESIZE;
	inf->parity   = DEFAULT_PARITY; // PARITY_NONE = 0, PARITY_ODD = 1, PARITY_EVEN = 2 FIXME: enum?
	inf->stopbits = DEFAULT_STOPBITS;
	inf->xOnXOff = false;
}
void jshSPIInitInfo(JshSPIInfo *inf) {
	inf->baudRate = 100000;
	inf->baudRateSpec = 0;
	inf->pinSCK = PIN_UNDEFINED;
	inf->pinMISO = PIN_UNDEFINED;
	inf->pinMOSI = PIN_UNDEFINED;
	inf->spiMode = SPIF_SPI_MODE_0;
	inf->spiMSB = true; // MSB first is default
}
void jshI2CInitInfo(JshI2CInfo *inf) {
	inf->pinSCL = PIN_UNDEFINED;
	inf->pinSDA = PIN_UNDEFINED;
	inf->slaveAddr = (char)-1; // master
	inf->bitrate = 50000; // Is what we used - shouldn't it be 100k?
}
// ----------------------------------------------------------------------------

IOEventFlags pinToEVEXTI(Pin pin) {
  return (IOEventFlags)0;
}


// ----------------------------------------------------------------------------
void jshInit() {
  jshInitDevices();
//  Serial.begin(DEFAULT_BAUD_RATE);
}

void jshReset() {
}

void jshKill() {
}

void jshIdle() {
//	os_printf("jshIdle");
//   while (Serial.available() > 0) {
//      jshPushIOCharEvent(EV_SERIAL1, Serial.read());
//   }
}

// ----------------------------------------------------------------------------

int jshGetSerialNumber(unsigned char *data, int maxChars) {
  const char *code = "ESP8266";
  strncpy((char *)data, code, maxChars);
  return strlen(code);
}

// ----------------------------------------------------------------------------

void jshInterruptOff() {
}

void jshInterruptOn() {
}

void jshDelayMicroseconds(int microsec) {
	os_printf("jshDelayMicroseconds %d\n", microsec);
	ets_delay_us(microsec);
}

static uint8_t PERIPHS[] = {
  PERIPHS_IO_MUX_GPIO0_U - PERIPHS_IO_MUX,
  PERIPHS_IO_MUX_U0TXD_U - PERIPHS_IO_MUX,
  PERIPHS_IO_MUX_GPIO2_U - PERIPHS_IO_MUX,
  PERIPHS_IO_MUX_U0RXD_U - PERIPHS_IO_MUX,
  PERIPHS_IO_MUX_GPIO4_U - PERIPHS_IO_MUX,
  PERIPHS_IO_MUX_GPIO5_U - PERIPHS_IO_MUX,
  PERIPHS_IO_MUX_SD_CLK_U - PERIPHS_IO_MUX,
  PERIPHS_IO_MUX_SD_DATA0_U - PERIPHS_IO_MUX,
  PERIPHS_IO_MUX_SD_DATA1_U - PERIPHS_IO_MUX,
  PERIPHS_IO_MUX_SD_DATA2_U - PERIPHS_IO_MUX,
  PERIPHS_IO_MUX_SD_DATA3_U - PERIPHS_IO_MUX,
  PERIPHS_IO_MUX_SD_CMD_U - PERIPHS_IO_MUX,
  PERIPHS_IO_MUX_MTDI_U - PERIPHS_IO_MUX,
  PERIPHS_IO_MUX_MTCK_U - PERIPHS_IO_MUX,
  PERIPHS_IO_MUX_MTMS_U - PERIPHS_IO_MUX,
  PERIPHS_IO_MUX_MTDO_U - PERIPHS_IO_MUX
};
#define FUNC_SPI 1
#define FUNC_GPIO 3
#define FUNC_UART 4
static uint8_t function(JshPinState state) {
	switch (state) {
		case JSHPINSTATE_GPIO_OUT:
		case JSHPINSTATE_GPIO_OUT_OPENDRAIN:
		case JSHPINSTATE_GPIO_IN:
		case JSHPINSTATE_GPIO_IN_PULLUP:
		case JSHPINSTATE_GPIO_IN_PULLDOWN: return FUNC_GPIO;
		case JSHPINSTATE_USART_OUT:
		case JSHPINSTATE_USART_IN: return FUNC_UART;
		case JSHPINSTATE_I2C: return FUNC_SPI;
		case JSHPINSTATE_AF_OUT:
		case JSHPINSTATE_AF_OUT_OPENDRAIN:
		case JSHPINSTATE_DAC_OUT:
		case JSHPINSTATE_ADC_IN:
		default: return 0;
	}
}
void jshPinSetState(Pin pin, JshPinState state) {
//	os_printf("jshPinSetState %d, %d\n", pin, state);
	
	assert(pin < 16);
	int periph = PERIPHS_IO_MUX + PERIPHS[pin];
	PIN_PULLUP_DIS(periph);
	//PIN_PULLDWN_DIS(periph);

	uint8_t primary_func = pin < 6 ? (PERIPHS_IO_MUX_U0TXD_U == pin || PERIPHS_IO_MUX_U0RXD_U == pin) ? FUNC_UART : FUNC_GPIO : 0;
	uint8_t select_func = function(state);
	PIN_FUNC_SELECT(periph, primary_func == select_func ? 0 : select_func);

	switch (state) {
		case JSHPINSTATE_GPIO_OUT:
		case JSHPINSTATE_GPIO_OUT_OPENDRAIN:
		//case JSHPINSTATE_AF_OUT:
		//case JSHPINSTATE_AF_OUT_OPENDRAIN:
		//case JSHPINSTATE_USART_OUT:
		//case JSHPINSTATE_DAC_OUT:
			gpio_output_set(0, 0x1<<pin, 0x1<<pin, 0);
			break;
		case JSHPINSTATE_ADC_IN:
		case JSHPINSTATE_USART_IN:
		case JSHPINSTATE_I2C:
			PIN_PULLUP_EN(periph);
			break;
		case JSHPINSTATE_GPIO_IN_PULLUP: PIN_PULLUP_EN(periph);
		case JSHPINSTATE_GPIO_IN_PULLDOWN: if (JSHPINSTATE_GPIO_IN_PULLDOWN == pin) PIN_PULLDWN_EN(periph);
		case JSHPINSTATE_GPIO_IN:
			gpio_output_set(0, 0, 0, 0x1<<pin);
			break;
		default:;
	}
}

JshPinState jshPinGetState(Pin pin) {
  os_printf("jshPinGetState %d\n", pin);
  return JSHPINSTATE_UNDEFINED;
}

void jshPinSetValue(Pin pin, bool value) {
//	os_printf("jshPinSetValue %d, %d\n", pin, value);

  GPIO_OUTPUT_SET(pin, value);
}

bool jshPinGetValue(Pin pin) {
//	os_printf("jshPinGetValue %d, %d\n", pin, GPIO_INPUT_GET(pin));

  return GPIO_INPUT_GET(pin);
}

bool jshIsDeviceInitialised(IOEventFlags device) {
  os_printf("jshIsDeviceInitialised %d\n", device);
  return true;
}

bool jshIsUSBSERIALConnected() {
  os_printf("jshIsUSBSERIALConnected\n");
  return true;
}

//----------------------
/*
static JsSysTime jshGetTimeForSecond() {
#ifdef USE_RTC
	return (JsSysTime)JSSYSTIME_SECOND;
#else
	return (JsSysTime)getSystemTimerFreq();
#endif
}

JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms) {
	return (JsSysTime)((ms*(JsVarFloat)jshGetTimeForSecond())/1000);
}

JsVarFloat jshGetMillisecondsFromTime(JsSysTime time) {
	return ((JsVarFloat)time)*1000/(JsVarFloat)jshGetTimeForSecond();
}

#ifdef USE_RTC
#ifdef STM32F1
unsigned short rtcHighBits = 0;
unsigned int rtcLastCall = 0;
#endif

JsSysTime jshGetRTCSystemTime() {
#ifdef STM32F1
	volatile uint16_t dl,ch,cl,cl1;
	do {
		cl1 = RTC->CNTL;
		dl = RTC->DIVL;
		ch = RTC->CNTH;
		cl = RTC->CNTL;
	} while(cl1!=cl);
	
	unsigned int chl = (((unsigned int)ch)<<16) | (unsigned int)cl;
	if (chl < rtcLastCall) {
		rtcLastCall = chl;
		rtcHighBits++;
	}
	JsSysTime c = chl | (((JsSysTime)rtcHighBits)<<32);
	
#else
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;
	uint16_t dl = (uint16_t)RTC_GetSubSecond(); // get first, as this freezes the time + date
	RTC_GetTime(RTC_Format_BIN, &time);
	RTC_GetDate(RTC_Format_BIN, &date);
	
	CalendarDate cdate;
	TimeInDay ctime;
	cdate.day = date.RTC_Date;
	cdate.month = date.RTC_Month;
	cdate.year = 2000+date.RTC_Year;
	cdate.dow = date.RTC_WeekDay%7;
	ctime.daysSinceEpoch = fromCalenderDate(&cdate);
	ctime.zone = 0;
	ctime.ms = 0;
	ctime.sec = time.RTC_Seconds;
	ctime.min = time.RTC_Minutes;
	ctime.hour = time.RTC_Hours;
	
	JsSysTime c = (JsSysTime)(fromTimeInDay(&ctime)/1000);
#endif
	return (((JsSysTime)c) << JSSYSTIME_SECOND_SHIFT) | (JsSysTime)((((unsigned int)jshRTCPrescaler - (unsigned int)(dl+1))*(unsigned int)jshRTCPrescalerReciprocal) >> RTC_PRESCALER_RECIPROCAL_SHIFT);
}
#endif

JsSysTime jshGetSystemTime() {
#ifdef USE_RTC
	if (ticksSinceStart<=RTC_INITIALISE_TICKS)
		return jshGetRTCSystemTime(); // Clock hasn't stabilised yet, just use whatever RTC value we currently have
	if (hasSystemSlept) {
		// reset SysTick counter. This will hopefully cause it
		// to fire off a SysTick IRQ, which will reset lastSysTickTime
		SysTick->VAL = 0; // this doesn't itself fire an IRQ it seems
		jshDoSysTick();
	}
	// Try and fix potential glitch caused by rollover of SysTick
	JsSysTime last1, last2;
	unsigned int avr1,avr2;
	unsigned int sysTick;
	do {
		avr1 = smoothAverageSysTickTime;
		last1 = smoothLastSysTickTime;
		sysTick = SYSTICK_RANGE - SysTick->VAL;
		last2 = smoothLastSysTickTime;
		avr2 = smoothAverageSysTickTime;
	} while (last1!=last2 || avr1!=avr2);
	// Now work out time...
	return last2 + (((JsSysTime)sysTick*(JsSysTime)avr2)/SYSTICK_RANGE);
#else
	JsSysTime major1, major2, major3, major4;
	unsigned int minor;
	do {
		major1 = SysTickMajor;
		major2 = SysTickMajor;
		minor = SysTick->VAL;
		major3 = SysTickMajor;
		major4 = SysTickMajor;
	} while (major1!=major2 || major2!=major3 || major3!=major4);
	return major1 - (JsSysTime)minor;
#endif
}

void jshSetSystemTime(JsSysTime newTime) {
	jshInterruptOff();
	// NOTE: Subseconds are not set here
#ifdef USE_RTC
	
#ifdef STM32F1
	rtcLastCall = (unsigned int)(newTime>>JSSYSTIME_SECOND_SHIFT);
	rtcHighBits = (unsigned short)(newTime>>(JSSYSTIME_SECOND_SHIFT+32));
	RTC_SetCounter(rtcLastCall);
	
	RTC_WaitForLastTask();
#else // !STM32F1
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;
	
	
	TimeInDay ctime = getTimeFromMilliSeconds((JsVarFloat)newTime * 1000 / JSSYSTIME_SECOND);
	CalendarDate cdate = getCalendarDate(ctime.daysSinceEpoch);
	
	date.RTC_Date = (uint8_t)cdate.day;
	date.RTC_Month = (uint8_t)cdate.month;
	date.RTC_Year = (uint8_t)(cdate.year - 2000);
	date.RTC_WeekDay = (uint8_t)(cdate.dow + 1);
	
	time.RTC_Seconds = (uint8_t)ctime.sec;
	time.RTC_Minutes = (uint8_t)ctime.min;
	time.RTC_Hours = (uint8_t)ctime.hour;
	time.RTC_H12 = 0;
	
	RTC_SetTime(RTC_Format_BIN, &time);
	RTC_SetDate(RTC_Format_BIN, &date);
	RTC_WaitForSynchro();
#endif // !STM32F1
	
	
	hasSystemSlept = true;
#else // !USE_RTC
	SysTickMajor = newTime;
#endif // !USE_RTC
	jshInterruptOn();
	jshGetSystemTime(); // force update of the time
}
 */
//----------------------


// setInterval(function() { console.log('1'); }, 1000);
// setTimeout(function() { console.log('1'); }, 1000);
/*JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms) {
	return (JsSysTime)((ms*(JsVarFloat)jshGetTimeForSecond())/1000);
}

JsVarFloat jshGetMillisecondsFromTime(JsSysTime time) {
	return ((JsVarFloat)time)*1000/(JsVarFloat)jshGetTimeForSecond();
}*/

JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms) {
  os_printf("jshGetTimeFromMilliseconds %lld\n", (JsSysTime)(ms * 1000.0f));
  return (JsSysTime)(ms * 1000.0f);
}

JsVarFloat jshGetMillisecondsFromTime(JsSysTime time) {
  os_printf("jshGetMillisecondsFromTime %lld\n", time);
  return (JsVarFloat)(time / 1000.0f);
}

JsSysTime jshGetSystemTime() { // in us
//	os_printf("jshGetSystemTime %d %d %d\n", system_get_time(), system_get_rtc_time(), system_rtc_clock_cali_proc());
	
  return ((JsSysTime)system_get_time());
}

void jshSetSystemTime(JsSysTime time) {
  os_printf("SetSystemTime: %lld\n", time);
}

// ----------------------------------------------------------------------------

JsVarFloat jshPinAnalog(Pin pin) {
  return (JsVarFloat)system_adc_read();
}

int jshPinAnalogFast(Pin pin) {
	return NAN;
}

JshPinFunction jshPinAnalogOutput(Pin pin, JsVarFloat value, JsVarFloat freq) { // if freq<=0, the default is used
  return 0;
}

void jshSetOutputValue(JshPinFunction func, int value) {
  os_printf("jshSetOutputValue %d %d\n", func, value);
}

void jshEnableWatchDog(JsVarFloat timeout) {
  os_printf("jshEnableWatchDog %0.3f\n", timeout);
}

bool jshGetWatchedPinState(IOEventFlags device) {
  //os_printf("jshGetWatchedPinState %d", device);
	return false;
}

void jshPinPulse(Pin pin, bool value, JsVarFloat time) {
  if (jshIsPinValid(pin)) {
    jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
    jshPinSetValue(pin, value);
//    delayMicroseconds(time*1000000);
    jshPinSetValue(pin, !value);
  } else jsError("Invalid pin!");
}

bool jshCanWatch(Pin pin) {
  return false;
}

IOEventFlags jshPinWatch(Pin pin, bool shouldWatch) {
  if (jshIsPinValid(pin)) {
  } else jsError("Invalid pin!");
  return EV_NONE;
}

JshPinFunction jshGetCurrentPinFunction(Pin pin) {
  os_printf("jshGetCurrentPinFunction %d\n", pin);
  return JSH_NOTHING;
}

bool jshIsEventForPin(IOEvent *event, Pin pin) {
  return IOEVENTFLAGS_GETTYPE(event->flags) == pinToEVEXTI(pin);
}

void jshUSARTSetup(IOEventFlags device, JshUSARTInfo *inf) {
}

/** Kick a device into action (if required). For instance we may need
 * to set up interrupts */
void jshUSARTKick(IOEventFlags device) {
//   int c;
//   while ((c = jshGetCharToTransmit(EV_SERIAL1)) >= 0) {
//      Serial.write((char)c);
//   }
}

void jshSPISetup(IOEventFlags device, JshSPIInfo *inf) {
}

/** Send data through the given SPI device (if data>=0), and return the result
 * of the previous send (or -1). If data<0, no data is sent and the function
 * waits for data to be returned */
int jshSPISend(IOEventFlags device, int data) {
	return NAN;
}

/** Send 16 bit data through the given SPI device. */
void jshSPISend16(IOEventFlags device, int data) {
  jshSPISend(device, data>>8);
  jshSPISend(device, data&255);
}

/** Set whether to send 16 bits or 8 over SPI */
void jshSPISet16(IOEventFlags device, bool is16) {
}

/** Wait until SPI send is finished, */
void jshSPIWait(IOEventFlags device) {
}

void jshI2CSetup(IOEventFlags device, JshI2CInfo *inf) {
}

void jshI2CWrite(IOEventFlags device, unsigned char address, int nBytes, const unsigned char *data, bool sendStop) {
}

void jshI2CRead(IOEventFlags device, unsigned char address, int nBytes, unsigned char *data, bool sendStop) {
}


void jshSaveToFlash() {
  os_printf("jshSaveToFlash\n");
}

void jshLoadFromFlash() {
  os_printf("jshLoadFromFlash\n");
}

bool jshFlashContainsCode() {
  os_printf("jshFlashContainsCode\n");
  return false;
}

/// Enter simple sleep mode (can be woken up by interrupts). Returns true on success
bool jshSleep(JsSysTime timeUntilWake) {
	if (timeUntilWake > 10000000) return false;
	os_printf("jshSleep %lld\n", timeUntilWake);
	ets_delay_us(timeUntilWake); return true;
//  return true;
	return false;
}

void jshUtilTimerDisable() {
	os_printf("jshUtilTimerDisable\n");
}

void jshUtilTimerReschedule(JsSysTime period) {
	os_printf("jshUtilTimerReschedule %lld\n", period);
}

void jshUtilTimerStart(JsSysTime period) {
	os_printf("jshUtilTimerStart %lld\n", period);
}

JsVarFloat jshReadTemperature() { return NAN; };
JsVarFloat jshReadVRef()  { return NAN; };

//}
