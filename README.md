# Espruino-on-ESP8266
This is a temporary project. A goal is to run the Espruino JavaScript (github.com/espruino) built with the Xtensa under esp-open-sdk (github.com/pfalcon/esp-open-sdk) directly on ESP8266 wifi module.

All the code changes for now are "dirty", source files and headers are in ./user/. Once it pass the tests, we are going to contribute to Espruino repository.

Current status: It passed initial test with jsVar and eval.

To do: Implement hardware specifics. First receive and execute JS code from UART.

# How to run?

Mac:

make flash

Linux:

esptool.py --port /dev/ttyUSB0 write_flash 0x00000 firmware/0x00000.bin 0x10000 firmware/0x10000.bin