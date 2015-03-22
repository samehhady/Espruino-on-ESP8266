# Espruino-on-ESP8266
This is a temporary project. A goal is to run the Espruino JavaScript (github.com/espruino) built with the Xtensa under esp-open-sdk (github.com/pfalcon/esp-open-sdk) directly on ESP8266 wifi module.

All the code changes for now are "dirty", source files and headers are in ./user/. Once it pass the tests, we are going to contribute to Espruino repository.

To do:
- Implement hardware specifics.
- Implement file system for loading JS files from flash.

Current status:
- Basic functionality for gpio Pins manipulation is implemented.
- It will evaluate JS code from serial port (115200 baud).

Example - toggle GPIO2:
var gpio2 = new Pin(2);
gpio2.write(!gpio2.read());

# How to run?

esptool.py --port /dev/ttyUSB0 write_flash 0x00000 firmware/0x00000.bin 0x10000 firmware/0x10000.bin
