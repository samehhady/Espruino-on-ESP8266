// auto-generated pin info file
// for board PICO_R1_2

#ifndef JSPININFO_H
#define JSPININFO_H

#include "jspin.h"

#define JSH_PIN_COUNT 36

#define JSH_PORTA_COUNT 16
#define JSH_PORTB_COUNT 16
#define JSH_PORTC_COUNT 16
#define JSH_PORTD_COUNT 0
#define JSH_PORTE_COUNT 0
#define JSH_PORTF_COUNT 0
#define JSH_PORTG_COUNT 0
#define JSH_PORTH_COUNT 2
#define JSH_PORTA_OFFSET 0
#define JSH_PORTB_OFFSET 16
#define JSH_PORTC_OFFSET 31
#define JSH_PORTD_OFFSET -1
#define JSH_PORTE_OFFSET -1
#define JSH_PORTF_OFFSET -1
#define JSH_PORTG_OFFSET -1
#define JSH_PORTH_OFFSET 34

#define JSH_PININFO_FUNCTIONS 4

typedef struct JshPinInfo {
  JsvPinInfoPort port;
  JsvPinInfoPin pin;
  JsvPinInfoAnalog analog; // TODO: maybe we don't need to store analogs separately
  JshPinFunction functions[JSH_PININFO_FUNCTIONS];
} PACKED_FLAGS JshPinInfo;

extern const JshPinInfo pinInfo[JSH_PIN_COUNT];

#endif // JSPININFO_H
