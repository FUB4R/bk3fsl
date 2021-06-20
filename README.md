# Black Knight: Sword of Rage - Flaming Speaker Lights (`bk3fsl`)

More info in this thread: https://pinside.com/pinball/forum/topic/bksor-flaming-speaker-mod-diy

Demo: https://vimeo.com/565235614

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

## Hardware setup
> :warning: **DISCLAIMER:** Proceed at your own risk. This will void your warranty. Not endorsed by Stern. Author is not liable for any damage!

The game console needs to be wired to the Arduino's UART. For this there are 2 options:

* Use a ttyUSB device (USB-serial converter) which is supported by the game's kernel (FTDI chips are).
* Use the (unpopulated) CN2 connector on the carrier board.

> :warning: **The CN2 connector is wired to the i.MX6's UART which does not tolerate 5V !!**

The easiest option is probably the first one, use something like this with an Arduino Nano: https://www.hobbytronics.co.uk/ftdi-basic

## SD card configuration
The game console is disabled by default. To enable it, you must edit:
```
/etc/init.d/game_console
```
which on my game is a symlink to `spk/packages/spike-2_0_18/etc/init.d/game_console`.

If you want to use CN2 change it to:
```
CONSOLE_LOG=/dev/ttymxc0
CONSOLE_INPUT=/dev/ttymxc0
```

If you're using a USB-serial device, change it to:
```
CONSOLE_LOG=/dev/ttyUSB0
CONSOLE_INPUT=/dev/ttyUSB0
```

> :warning: **If the USB-serial device is disconnected or not recognised, the game won't boot!** (It will hang on the Stern logo with "Initializing...".)
