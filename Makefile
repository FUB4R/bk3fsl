# Arduino Make file. Refer to https://github.com/sudar/Arduino-Makefile


BOARD_TAG     = nano328
USER_LIB_PATH = $(shell pwd)
ARDUINO_LIBS  = FastLED

# Code sets the serial port to 115200 - match here.
AVRDUDE_ARD_BAUDRATE=115200

include Arduino-Makefile/Arduino.mk
