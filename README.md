# ArduinoDonut

This Arduino sketch renders a rotating donut to the default serial port (`Serial`).

It is based on Andy Sloane's float-less `donut.c` code, which can be found [here](https://www.a1k0n.net/2021/01/13/optimizing-donut.html).

## Supported hardware

Theoretically, this sketch should run on any Arduino-compatible MCU with at least 2K of RAM (e.g. ATmega328/Arduino Uno).

In practice, it has only been confirmed to be working on the ESP32, and more work is needed to get it working on AVR MCUs.

## Installation/Usage

Upload the `donut` sketch in this repo using Arduino IDE.

Additionally, the first lines of `donut.ino` contain user-configurable parameters for rendering and outputting.

## Contributions

Contributions via pull requests or issues are welcome.
