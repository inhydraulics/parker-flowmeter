# Parker Flow Meter Signal Converter Board Recovery with ATtiny 4313 MCU
## About this project
This is a simple reciprocal frequency measurement routine that was implemented on ATtiny4313 MCU.
It uses INT0 interrupt to count pulses, and two timers for time measurement.
The objective was to measure frequency in the range of 8-900 Hz with at least 12 bit precision, 
then run the result through a linearization procedure and send it to a 12-bit LTC1257 DAC.
More details about this project at:
https://www.insanehydraulics.com/backengthis/flowmeterrecovery1.html
The project was built using Microchip Studio (Version. 7.0.2542), with the AVR/GNU C compiler.
