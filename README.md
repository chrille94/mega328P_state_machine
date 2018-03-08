# ATmega328p State Machine
4-state state machine implemented on an Atmel ATmega328p microcontroller. Programmed in C.

The state machine has four states; Red, Green, Blue and Yellow. The hardware consists of an ATmega328p MCU connected to two active low pushbuttons, four LEDs (one for each state) and a 7-segment display connected via a 74HC595 shift register.

# Usage
Pressing button 1 cycles between the Red, Green and Blue states in that order. I.e. if in the Red state, pressing button 1 takes you to state Green. Pressing again takes you to state Blue. Pressing again takes you back to state Red. Pressing Button 2 in any of the aforementionend states takes the state machine into state Yellow. At the same time, a 5 second timer is started and the countdown is showed on the 7-segment display. When the count reaches 0, the display is turned off and the state macine returns to state Red. The Yellow state can also be exited prematurely by pressing button 2 again. This will also take the state machine into state Red.

# Possible future improvements
Add debouncing for the two buttons.
