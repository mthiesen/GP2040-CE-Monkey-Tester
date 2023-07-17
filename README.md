# GP2040-CE-Monkey-Tester

This is a small program for the Raspberry Pi Pico to help with automated testing of the GP2040-CE firmware. It pulls the Pico's GPIOs to ground semi-randomly. Connecting the GPIO pins to button pins of a GP2040-CE device mimics a monkey randomly mashing the controller buttons.

# Usage

- Build and flash the UF2 on a Raspberry Pi Pico
- Wire a GPIO pin of the monkey device to each button pin of the test device you want to include in your test
- Press the BOOTSEL button on the monkey device to start/stop the test