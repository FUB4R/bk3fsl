#!/bin/bash
make -f makeEspArduino/makeEspArduino.mk ESP_ROOT=../arduino-esp32/ CHIP=esp32 "$@"
