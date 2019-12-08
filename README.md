# Conways-Game-of-Life-for-ESP32-and-SmartMatrix
An implementation of Conway's Game of Life using the ESP32 IOT processor and SmartMatrix-teensy

Credit to Gareth for the GoL generational code (http://myrobotlab.org/content/eps32-life-life-conways-life).

This version uses the SmartMatrix-teensy branch which provides ESP32 functionality.  The ESP32 is wired directly to the board using the wiring diagram provided in MatrixHardware_V0.h under the GPIOPINOUT ESP_FORUM_PINOUT.  (https://github.com/pixelmatix/SmartMatrix/blob/teensylc/src/MatrixHardware_ESP32_V0.h#L62)

The board used is an Alibaba P3 Matrix from th SRYLED Store (https://www.aliexpress.com/item/32728985432.html?spm=a2g0s.9042311.0.0.27424c4daQTm6s)  Power is provided from a bench 5V power supply (e.g. not through the ESP32.)

The Game of Life is implemented in the background layer, and the 'Game Over' text is implemented in the scrolling layer.  I plan on using this as a background on a clock for my workshop with time/temp scrolling on top.
