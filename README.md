# ArduinoDonut

This Arduino sketch renders a rotating donut to the default serial port (`Serial`).

It is based on Andy Sloane's add-and-shift `donut.c` code, which can be found [here](https://twitter.com/a1k0n/status/1716306717196030290).

## Supported hardware

Theoretically, this sketch should run on any Arduino-compatible MCU with at least 4K of Flash memory (i.e. program storage space) and 256 bytes of RAM (e.g. as low as ATtiny45/85, or ATmega328/Arduino Uno).

This sketch has been confirmed to be working on the ESP32 and Arduino Mega.

## Installation/Usage

Upload the `donut` sketch in this repo using Arduino IDE.

Additionally, the first lines of `donut.ino` contain user-configurable parameters for rendering and outputting.

## Contributions

Contributions via pull requests or issues are welcome.
