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
//	jsiConsolePrintf("jshIdle");
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
	if (0 < microsec) ets_delay_us(microsec);
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
//	jsiConsolePrintf("jshPinSetState %d, %d\n", pin, state);
	
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
  jsiConsolePrintf("jshPinGetState %d\n", pin);
  return JSHPINSTATE_UNDEFINED;
}

void jshPinSetValue(Pin pin, bool value) {
//	jsiConsolePrintf("jshPinSetValue %d, %d\n", pin, value);

  GPIO_OUTPUT_SET(pin, value);
}

bool jshPinGetValue(Pin pin) {
//	jsiConsolePrintf("jshPinGetValue %d, %d\n", pin, GPIO_INPUT_GET(pin));

  return GPIO_INPUT_GET(pin);
}

bool jshIsDeviceInitialised(IOEventFlags device) {
  jsiConsolePrintf("jshIsDeviceInitialised %d\n", device);
  return true;
}

bool jshIsUSBSERIALConnected() {
  jsiConsolePrintf("jshIsUSBSERIALConnected\n");
  return true;
}

JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms) {
//	jsiConsolePrintf("jshGetTimeFromMilliseconds %d, %f\n", (JsSysTime)(ms * 1000.0), ms);
	return (JsSysTime)(ms * 1000.0 + 0.5);
}

JsVarFloat jshGetMillisecondsFromTime(JsSysTime time) {
//	jsiConsolePrintf("jshGetMillisecondsFromTime %d, %f\n", time, (JsVarFloat)time / 1000.0);
	return (JsVarFloat)time / 1000.0;
}

JsSysTime jshGetSystemTime() { // in us
	uint32 t = system_get_time();
  JsSysTime time = (JsSysTime)t;
//  jsiConsolePrintf("jshGetSystemTime 1 %d, %d, %d\n", t, time, (uint32)time);
//  os_printf("jshGetSystemTime 2 %d, %lld, %d\n", t, time, (uint32)time);
  return time;
}

void jshSetSystemTime(JsSysTime time) {
  jsiConsolePrintf("SetSystemTime: %d\n", time);
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
  jsiConsolePrintf("jshSetOutputValue %d %d\n", func, value);
}

void jshEnableWatchDog(JsVarFloat timeout) {
  jsiConsolePrintf("jshEnableWatchDog %0.3f\n", timeout);
}

bool jshGetWatchedPinState(IOEventFlags device) {
  //jsiConsolePrintf("jshGetWatchedPinState %d", device);
	return false;
}

void jshPinPulse(Pin pin, bool value, JsVarFloat time) {
  if (jshIsPinValid(pin)) {
    jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
    jshPinSetValue(pin, value);
	jshDelayMicroseconds(jshGetTimeFromMilliseconds(time));
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
  jsiConsolePrintf("jshGetCurrentPinFunction %d\n", pin);
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
  jsiConsolePrintf("jshSaveToFlash\n");
}

void jshLoadFromFlash() {
  jsiConsolePrintf("jshLoadFromFlash\n");
}

bool jshFlashContainsCode() {
  jsiConsolePrintf("jshFlashContainsCode\n");
  return false;
}

/// Enter simple sleep mode (can be woken up by interrupts). Returns true on success
bool jshSleep(JsSysTime timeUntilWake) {
	int time = (int)timeUntilWake;
	os_printf("jshSleep %d\n", time);
	jshDelayMicroseconds(time);
	return true;
}

void jshUtilTimerDisable() {
	jsiConsolePrintf("jshUtilTimerDisable\n");
}

void jshUtilTimerReschedule(JsSysTime period) {
	jsiConsolePrintf("jshUtilTimerReschedule %d\n", period);
}

void jshUtilTimerStart(JsSysTime period) {
	jsiConsolePrintf("jshUtilTimerStart %d\n", period);
}

JsVarFloat jshReadTemperature() { return NAN; };
JsVarFloat jshReadVRef()  { return NAN; };

//}
