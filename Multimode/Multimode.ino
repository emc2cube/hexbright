/* hexbright firmware Multimode - v1.1  Apr 16, 2013 - https://github.com/emc2cube/hexbright
 
 Custom firmware for hexbright flashlights http://www.hexbright.com 
 This firmware require the hexbright library. See https://github.com/dhiltonp/hexbright for installation instruction.
 Easy install: fork this repository as "hexbright-emc2cube" inside your local copy (or fork) of "dhiltonp/hexbright"
 
 Modes description:
 - Button presses cycle through off, low (25% power), medium (50% power), high (75% power) and max (100% power) modes. If you stay more than 5s in the same mode, the next short press will turn of the light (can be modified).
 - In any mode, hold the button down for more than 500ms, and the light will fade up and down. Release the button to hold the current brightness. Another short press to turn off.
 - While holding the button down, give the light a firm tap to change to blink mode, another tap to change to dazzle mode, another tap to switch to morse mode and another to switch back to light fading mode. Release the button to stay in this mode. A short press will turn the light off. A long press (>500ms) will go back to the tap selection mode.
 
 */

// Include libraries
#include <math.h>
#include <Wire.h>
#include <hexbright.h>

// Define modes
#define MODE_OFF                0
#define MODE_LOW                1
#define MODE_MED                2
#define MODE_HIGH               3
#define MODE_MAX                4
#define MODE_FADE               5
#define MODE_ON                 6
#define MODE_BLINKING_PREVIEW   7
#define MODE_BLINKING           8
#define MODE_DAZZLING_PREVIEW   9
#define MODE_DAZZLING           10
#define MODE_MORSE_PREVIEW      11
#define MODE_MORSE              12


// State
hexbright hb;
byte mode;// = MODE_OFF;
char brightness_direction = -1;

// Morse Code: message
char message[] = "SOS";
// Morse Code: duration of a dot (in ms)
int millisPerBeat = 150;
// High byte = length
// Low byte  = morse code, LSB first, 0=dot 1=dash
word morse[] = {
  0x0202, // A .-
  0x0401, // B -...
  0x0405, // C -.-.
  0x0301, // D -..
  0x0100, // E .
  0x0404, // F ..-.
  0x0303, // G --.
  0x0400, // H ....
  0x0200, // I ..
  0x040E, // J .---
  0x0305, // K -.-
  0x0402, // L .-..
  0x0203, // M --
  0x0201, // N -.
  0x0307, // O ---
  0x0406, // P .--.
  0x040B, // Q --.-
  0x0302, // R .-.
  0x0300, // S ...
  0x0101, // T -
  0x0304, // U ..-
  0x0408, // V ...-
  0x0306, // W .--
  0x0409, // X -..-
  0x040D, // Y -.--
  0x0403, // Z --..
  0x051F, // 0 -----
  0x051E, // 1 .----
  0x051C, // 2 ..---
  0x0518, // 3 ...--
  0x0510, // 4 ....-
  0x0500, // 5 .....
  0x0501, // 6 -....
  0x0503, // 7 --...
  0x0507, // 8 ---..
  0x050F, // 9 ----.
};

void setup()
{
  hb.init_hardware();
}

void loop()
{
  hb.update();
  hb.print_charge(GLED);
  static unsigned long lastTapTime, go_off;
  unsigned long time = millis();

  // Register accelerometer taps
  byte tapped = 0;
  if (mode == MODE_FADE || mode == MODE_BLINKING_PREVIEW || mode == MODE_DAZZLING_PREVIEW || mode == MODE_MORSE_PREVIEW ) // Track taps only when we are expecting them
  {
    if (time-lastTapTime>25) // Prevent "tap firing" and only pass 1 tap by 25ms period.
    {
      lastTapTime = time;
      tapped = (hb.tapped());
      if (tapped) Serial.println("Tap!");
    }
  }

  // Do whatever this mode does
  switch (mode)
  {
  case MODE_OFF: // Light is off
    if (hb.button_pressed_time()>500) // Held
    {
      Serial.println("Mode = fade");
      mode = MODE_FADE;
    }
    else if (hb.button_just_released())
    { 
      Serial.println("Mode = low");
      hb.set_light(CURRENT_LEVEL, 250, NOW);
      mode = MODE_LOW; 
    }
    break;
  case MODE_LOW: // 25% power
    if (hb.button_released_time()>5000) // If we stay more than 5s in the same mode assume that next click will be to turn off
    {
      go_off = 1;
    }
    if (hb.button_pressed_time()>500) // Held
    {
      Serial.println("Mode = fade");
      mode = MODE_FADE;
    }
    else if (hb.button_just_released())
    {
      if (go_off == 1)
      {
        Serial.println("Mode = off");
        go_off = 0;
        hb.set_light(0, OFF_LEVEL, NOW);
        mode = MODE_OFF;
      }
      else
      {
        Serial.println("Mode = med");
        hb.set_light(CURRENT_LEVEL, 500, NOW);
        mode = MODE_MED;
      }
    }
    break;
  case MODE_MED: // 50% power
    if (hb.button_released_time()>5000) // If we stay more than 5s in the same mode assume that next click will be to turn off
    {
      go_off = 1;
    }
    if (hb.button_pressed_time()>500) // Held
    {
      Serial.println("Mode = fade");
      mode = MODE_FADE;
    }
    else if (hb.button_just_released())
    {
      if (go_off == 1)
      {
        Serial.println("Mode = off");
        go_off = 0;
        hb.set_light(0, OFF_LEVEL, NOW);
        mode = MODE_OFF;
      }
      else
      {
        Serial.println("Mode = high");
        hb.set_light(CURRENT_LEVEL, 750, NOW);
        mode = MODE_HIGH;
      }
    }
    break;
  case MODE_HIGH: // 75% power
    if (hb.button_released_time()>5000) // If we stay more than 5s in the same mode assume that next click will be to turn off
    {
      go_off = 1;
    }
    if (hb.button_pressed_time()>500) // Held
    {
      Serial.println("Mode = fade");
      mode = MODE_FADE;
    }
    else if (hb.button_just_released())
    {
      if (go_off == 1)
      {
        Serial.println("Mode = off");
        go_off = 0;
        hb.set_light(0, OFF_LEVEL, NOW);
        mode = MODE_OFF;
      }
      else
      {
        Serial.println("Mode = max");
        hb.set_light(CURRENT_LEVEL, MAX_LEVEL, NOW);
        mode = MODE_MAX;
      }
    }
    break;
  case MODE_MAX: // 100% power
    if (hb.button_pressed_time()>500) // Held
    {
      Serial.println("Mode = fade");
      mode = MODE_FADE;
    }
    else if (hb.button_just_released())
    { 
      Serial.println("Mode = off");
      hb.set_light(0, OFF_LEVEL, NOW);
      mode = MODE_OFF;
    }
    break;
  case MODE_FADE: // Cycle light intensity
    if (hb.button_pressed())
    {
      if (!hb.light_change_remaining())
        if (brightness_direction<0)
        { // the light is low, go up from here
          hb.set_light(CURRENT_LEVEL, 1000, 1000-(hb.get_light_level()));  // go from low to high over 1 second
          brightness_direction = 1;
        }
        else
        {
          hb.set_light(CURRENT_LEVEL, 1, hb.get_light_level()); // go from high to low over 1 second
          brightness_direction = -1;
        }
      if (tapped)
      {
        Serial.println("Mode = blinking_preview");
        mode = MODE_BLINKING_PREVIEW;
      }
    }
    else // released!
    { // we backtrack a little to help compensate for human reaction time
      int brightness = hb.get_light_level()+(50*brightness_direction);
      brightness = brightness<1 ? 1 : brightness; // if our adjust took us to 0, undo.
      Serial.println("Mode = on");
      hb.set_light(CURRENT_LEVEL, brightness, 50);
      mode = MODE_ON;
    }
    break;
  case MODE_ON: // Fade stopped, keep current brightness
    if (!hb.button_pressed() && hb.button_just_released())
    {
      Serial.println("Mode = off");
      hb.set_light(0, OFF_LEVEL, NOW);
      mode = MODE_OFF;
    }
    else if (hb.button_pressed() && hb.button_pressed_time()>500) // Held
    {
      Serial.println("Mode = fade");
      mode = MODE_FADE;
      // so we continue going the same way as before
      brightness_direction = -brightness_direction;
    }
    break;
  case MODE_BLINKING_PREVIEW: // Blinking preview
    if (hb.button_pressed())
    {
      static int i = 0;
      i = (i+1)%40;
      if (i==0)
        hb.set_light(MAX_LEVEL, 0, 50);
      if (tapped)
      {
        Serial.println("Mode = dazzling_preview");
        mode = MODE_DAZZLING_PREVIEW;
      }
    }
    else // released!
    {
      Serial.println("Mode = blinking");
      mode = MODE_BLINKING;
    }
    break;
  case MODE_BLINKING: // Blinking
    if (!hb.button_pressed())
    {
      static int i = 0;
      i = (i+1)%40;
      if (i==0)
        hb.set_light(MAX_LEVEL, 0, 50);
    }
    if (!hb.button_pressed() && hb.button_just_released())
    {
      Serial.println("Mode = off");
      hb.set_light(0, OFF_LEVEL, NOW);
      mode = MODE_OFF;
    }
    else if (hb.button_pressed() && hb.button_pressed_time()>500) // Held
    {
      Serial.println("Mode = blinking_preview");
      mode = MODE_BLINKING_PREVIEW;
    }
    break;
  case MODE_DAZZLING_PREVIEW: // Dazzling preview
    if (hb.button_pressed())
    {
      hb.set_light(0, (random(4)<1)*1000, NOW);
      if (tapped)
      {
        Serial.println("Mode = morse_preview");
        mode = MODE_MORSE_PREVIEW;
      }
    }
    else // released!
    {
      Serial.println("Mode = dazzling");
      mode = MODE_DAZZLING;
    }
    break;
  case MODE_DAZZLING: // Dazzling
    if (!hb.button_pressed())
    {
      hb.set_light(0, (random(4)<1)*1000, NOW);
    }
    if (!hb.button_pressed() && hb.button_just_released())
    {
      Serial.println("Mode = off");
      hb.set_light(0, OFF_LEVEL, NOW);
      mode = MODE_OFF;
    }
    else if (hb.button_pressed() && hb.button_pressed_time()>500) // Held
    {
      Serial.println("Mode = dazzling_preview");
      mode = MODE_DAZZLING_PREVIEW;
    }
    break;
  case MODE_MORSE_PREVIEW: // Morse preview
    if (hb.button_pressed())
    {
      if (!hb.light_change_remaining())
      { // we're not currently doing anything, start the next step
        static int current_character = 0;
        static char symbols_remaining = 0;
        static byte pattern = 0;

        if (current_character>=sizeof(message))
        { // we've hit the end of message.
          // reset the current_character to repeat message.
          current_character = 0;
          // Long delay before repeating.
          hb.set_light(0,0, millisPerBeat * 15);
        }

        if (symbols_remaining <= 0) // we're done printing our last character, get the next!
        {
          char ch = message[current_character];
          // Remap ASCII to the morse table
          if      (ch >= 'A' && ch <= 'Z') ch -= 'A';
          else if (ch >= 'a' && ch <= 'z') ch -= 'a';
          else if (ch >= '0' && ch <= '9') ch -= '0' - 26;
          else ch = -1; // character not in table

          if (ch>=0)
          {
            // Extract the symbols and length
            pattern = morse[ch] & 0x00FF;
            symbols_remaining = morse[ch] >> 8;
            // we count space (between dots/dashes) as a symbol to be printed;
            symbols_remaining *= 2;
          }
          current_character++;
        }

        if (symbols_remaining<=0)
        { // character was unrecognized, treat it as a space
          // 7 beats between words, but 3 have already passed
          // at the end of the last character
          hb.set_light(0,0, millisPerBeat * 4);
        }
        else if (symbols_remaining==1) 
        { // last symbol in character, long pause
          hb.set_light(0, 0, millisPerBeat * 3);
        }
        else if (symbols_remaining%2==1) 
        { // even symbol, print space!
          hb.set_light(0,0, millisPerBeat);
        }
        else if (pattern & 1)
        { // dash, 3 beats
          hb.set_light(MAX_LEVEL, MAX_LEVEL, millisPerBeat * 3);
          pattern >>= 1;
        }
        else 
        { // dot, by elimination
          hb.set_light(MAX_LEVEL, MAX_LEVEL, millisPerBeat);
          pattern >>= 1;
        }
        symbols_remaining--;
      }
      if (tapped)
      {
        Serial.println("Mode = mode_fade");
        mode = MODE_FADE;
      }
    }
    else // released!
    {
      Serial.println("Mode = morse");
      mode = MODE_MORSE;
    }
    break;
  case MODE_MORSE: // Morse message repeated
    if (!hb.button_pressed())
    {
      if (!hb.light_change_remaining())
      { // we're not currently doing anything, start the next step
        static int current_character = 0;
        static char symbols_remaining = 0;
        static byte pattern = 0;

        if (current_character>=sizeof(message))
        { // we've hit the end of message.
          // reset the current_character to repeat message.
          current_character = 0;
          // Long delay before repeating.
          hb.set_light(0,0, millisPerBeat * 15);
        }

        if (symbols_remaining <= 0) // we're done printing our last character, get the next!
        {
          char ch = message[current_character];
          // Remap ASCII to the morse table
          if      (ch >= 'A' && ch <= 'Z') ch -= 'A';
          else if (ch >= 'a' && ch <= 'z') ch -= 'a';
          else if (ch >= '0' && ch <= '9') ch -= '0' - 26;
          else ch = -1; // character not in table

          if (ch>=0)
          {
            // Extract the symbols and length
            pattern = morse[ch] & 0x00FF;
            symbols_remaining = morse[ch] >> 8;
            // we count space (between dots/dashes) as a symbol to be printed;
            symbols_remaining *= 2;
          }
          current_character++;
        }

        if (symbols_remaining<=0)
        { // character was unrecognized, treat it as a space
          // 7 beats between words, but 3 have already passed
          // at the end of the last character
          hb.set_light(0,0, millisPerBeat * 4);
        }
        else if (symbols_remaining==1) 
        { // last symbol in character, long pause
          hb.set_light(0, 0, millisPerBeat * 3);
        }
        else if (symbols_remaining%2==1) 
        { // even symbol, print space!
          hb.set_light(0,0, millisPerBeat);
        }
        else if (pattern & 1)
        { // dash, 3 beats
          hb.set_light(MAX_LEVEL, MAX_LEVEL, millisPerBeat * 3);
          pattern >>= 1;
        }
        else 
        { // dot, by elimination
          hb.set_light(MAX_LEVEL, MAX_LEVEL, millisPerBeat);
          pattern >>= 1;
        }
        symbols_remaining--;
      }
      if (tapped)
      {
        Serial.println("Mode = mode_fade");
        mode = MODE_FADE;
      }
    }
    if (!hb.button_pressed() && hb.button_just_released())
    {
      Serial.println("Mode = off");
      hb.set_light(0, OFF_LEVEL, NOW);
      mode = MODE_OFF;
    }
    else if (hb.button_pressed() && hb.button_pressed_time()>500) // Held
    {
      Serial.println("Mode = morse_preview");
      mode = MODE_MORSE_PREVIEW;
    }
    break;
  }

}

