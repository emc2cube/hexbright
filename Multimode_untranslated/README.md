hexbright firmware Multimode - v1.0  Apr 13, 2013 - https://github.com/emc2cube/hexbright
=========

Custom firmware for hexbright flashlights http://www.hexbright.com
This firmware DOES NOT require the hexbright library available at https://github.com/dhiltonp/hexbright and can be uploaded as is.

Modes description:
------------------
- Button presses cycle through off, low (20% power), medium (50% power), high (100% power) modes. If you stay more than 5s in the same mode, the next short press will turn of the light (from https://github.com/dblume/hexbright-factory).
- In any mode, hold the button down for more than 500ms, and the light will fade up and down. Release the button to hold the current brightness. Another short press to turn off.
- While holding the button down, give the light a firm tap to change to blink mode, another tap to change to dazzle mode, another tap to switch to morse mode and another to switch back to light fading mode. Release the button to stay in this mode. A short press will turn the light off. A long press (>500ms) will go back to the tap selection mode.
- After 10min inactivity (no button press or no movement detected) the light will turn off (can be modified or disabled).

To do:
------
Adding variable on the top of the program for easy user setup of long press delay, etc...
