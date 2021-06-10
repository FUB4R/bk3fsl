# Arduino Make file. Refer to https://github.com/sudar/Arduino-Makefile


BOARD_TAG     = nano
USER_LIB_PATH = $(shell pwd)
ARDUINO_LIBS  = FastLED

include Arduino-Makefile/Arduino.mk
