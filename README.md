# Arduino Snake
The Classic Snake Game - written for Arduino and an 8x8 LED Dot Matrix, but with a twist!
## Hardware Requirements
This project uses the Arduino Pro Mini, with the Atmel ATmega328 (5V, 16MHz), an 8x8 LED Dot Matrix, and two 74HC595 shift registers to drive the LED matrix.
## Code Portability
The code in this project is mostly portable to any size dot matrix, but the display code is not portable, and has been written under the assumption that the board size is 8x8.

Any other board size or differing number of shift registers used will require a code rework.
## Provided Circuit Diagram
The circuit diagram has been lifted from SunFounder's Dot-matrix Display tutorial, available on their website (under the tutorial: Super Kit V2.0 for Arduino) and is included in the base directory of this project as reference.
## Credit
Credit to SunFounder for the original 8x8 LED Dot Matrix display tutorial and for the circuit diagram

Email:support@sunfounder.com

Website:www.sunfounder.com