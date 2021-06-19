# Black Knight: Sword of Rage - Flaming Speaker Lights (`bk3fsl`)

More info in this thread:

* https://pinside.com/pinball/forum/topic/bksor-flaming-speaker-mod-diy

## Compiling
Make sure you cloned the repo with submodules (`--recursive`), otherwise you can fix it with:
```
git submodule update --init
```

On Ubuntu you can `apt install arduino` to get the SDK, then just type:
```
make
```

## Uploading
[Arduino-Makefile](https://github.com/sudar/Arduino-Makefile) has a bunch of options (try `make help`). This one does 99% of what you need (including rebuild):
```
make upload
```
If you need to specify another device (autodetect failed):
```
make DEVICE_PATH=/dev/ttyUSB0 upload
```

Note that baud rate 115200 is used here. If you need to change it, the programming baud rate is set by `AVRDUDE_ARD_BAUDRATE` in the `Makefile`.

## Libraries
Of course you can also do all of this in the official IDE... ;-) You will need the [FastLED](https://github.com/FastLED/FastLED) and [NeoHWSerial](https://github.com/SlashDevin/NeoHWSerial) libraries.
