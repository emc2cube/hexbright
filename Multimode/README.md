hexbright firmware Multimode - v1.1  Apr 16, 2013 - https://github.com/emc2cube/hexbright
=========

Custom firmware for hexbright flashlights http://www.hexbright.com 
This firmware require the hexbright library. See https://github.com/dhiltonp/hexbright for installation instruction.
Easy install: fork this repository as "hexbright-emc2cube" inside your local copy (or fork) of "dhiltonp/hexbright"

Modes description:
------------------
- Button presses cycle through off, low (25% power), medium (50% power), high (75% power) and max (100% power) modes. If you stay more than 5s in the same mode, the next short press will turn of the light (can be modified).
- In any mode, hold the button down for more than 500ms, and the light will fade up and down. Release the button to hold the current brightness. Another short press to turn off.
- While holding the button down, give the light a firm tap to change to blink mode, another tap to change to dazzle mode, another tap to switch to morse mode and another to switch back to light fading mode. Release the button to stay in this mode. A short press will turn the light off. A long press (>500ms) will go back to the tap selection mode.

To do:
------
Adding variable on the top of the program for easy user setup of long press delay, etc...
Detection of the "5s in the same mode" not very clean. Could probably be optimized using library functions. Ideally change it to a "2 short presses to switch off".
Detection of taps by hb.tapped() lead to a fire of 1-5 tap events (vibration after tap identified as taps?). Is there a cleaner way to restrict tap detection over a short period of time? 
Original 10min inactivity switch off not implemented. Unless a tracking of the last movement is implemented in the library this will probably not be enabled again.
