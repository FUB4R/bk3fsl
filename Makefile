# Arduino Make file. Refer to https://github.com/sudar/Arduino-Makefile

esp32:
	espmake32

BOARD_TAG     = nano
BOARD_SUB     = atmega328old
USER_LIB_PATH = $(shell pwd)
ARDUINO_LIBS  = FastLED NeoHWSerial

# Code sets the serial port to 115200 - match here.
AVRDUDE_ARD_BAUDRATE=115200

# Require with more recent arduino versions...
AVRDUDE_CONF = /usr/share/arduino/hardware/tools/avrdude.conf

include Arduino-Makefile/Arduino.mk
