hexbright custom firmware - https://github.com/emc2cube/hexbright
=========

Custom firmware for hexbright flashlights http://www.hexbright.com 
New firmwares will require the hexbright library. See https://github.com/dhiltonp/hexbright for installation instruction and on how to upload custom firmwares to your hexbright.
Easy install: fork this repository as "hexbright-emc2cube" inside your local copy (or fork) of "dhiltonp/hexbright"

Multimode:
----------
Updated version of Multimode, now use standard hexbright library.

- Button presses cycle through off, low (25% power), medium (50% power), high (75% power) and max (100% power) modes. If you stay more than 5s in the same mode, the next short press will turn of the light (can be modified).
- In any mode, hold the button down for more than 500ms, and the light will fade up and down. Release the button to hold the current brightness. Another short press to turn off.
- While holding the button down, give the light a firm tap to change to blink mode, another tap to change to dazzle mode, another tap to switch to morse mode and another to switch back to light fading mode. Release the button to stay in this mode. A short press will turn the light off. A long press (>500ms) will go back to the tap selection mode.

Multimode_untranslated:
-----------------------
First version of the Multimode program. Do not require the hexbright library.

- Button presses cycle through off, low (20% power), medium (50% power), high (100% power) modes. If you stay more than 5s in the same mode, the next short press will turn of the light (from https://github.com/dblume/hexbright-factory).
- In any mode, hold the button down for more than 500ms, and the light will fade up and down. Release the button to hold the current brightness. Another short press to turn off.
- While holding the button down, give the light a firm tap to change to blink mode, another tap to change to dazzle mode, another tap to switch to morse mode and another to switch back to light fading mode. Release the button to stay in this mode. A short press will turn the light off. A long press (>500ms) will go back to the tap selection mode.
- After 10min inactivity (no button press or no movement detected) the light will turn off (can be modified or disabled).
