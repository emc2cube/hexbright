/* hexbright firmware Multimode - v1.0  Apr 13, 2013 - https://github.com/emc2cube/hexbright
 
 Custom firmware for hexbright flashlights http://www.hexbright.com
 This firmware DOES NOT require the hexbright library available at https://github.com/dhiltonp/hexbright and can be uploaded as is.
 
 Modes description:
 - Button presses cycle through off, low (20% power), medium (50% power), high (100% power) modes. If you stay more than 5s in the same mode, the next short press will turn of the light (from https://github.com/dblume/hexbright-factory ).
 - In any mode, hold the button down for more than 500ms, and the light will fade up and down. Release the button to hold the current brightness. Another short press to turn off.
 - While holding the button down, give the light a firm tap to change to blink mode, another tap to change to dazzle mode, another tap to switch to morse mode and another to switch back to light fading mode. Release the button to stay in this mode. A short press will turn the light off. A long press (>500ms) will go back to the tap selection mode.
 - After 10min inactivity (no button press or no movement detected) the light will turn off (can be modified or disabled).
 
 */

#include <math.h>
#include <Wire.h>

// Settings
#define OVERTEMP                340
// Constants
#define ACC_ADDRESS             0x4C
#define ACC_REG_XOUT            0
#define ACC_REG_YOUT            1
#define ACC_REG_ZOUT            2
#define ACC_REG_TILT            3
#define ACC_REG_INTS            6
#define ACC_REG_MODE            7
// Pin assignments
#define DPIN_RLED_SW            2
#define DPIN_GLED               5
#define DPIN_PGOOD              7
#define DPIN_PWR                8
#define DPIN_DRV_MODE           9
#define DPIN_DRV_EN             10
#define DPIN_ACC_INT            3
#define APIN_TEMP               0
#define APIN_CHARGE             3
// Interrupts
#define INT_SW                  0
#define INT_ACC                 1
// Modes
#define MODE_POWERUP            0
#define MODE_OFF                1
#define MODE_LOW                2
#define MODE_MED                3
#define MODE_HIGH               4
#define MODE_FADE               5
#define MODE_ON                 6
#define MODE_BLINKING           7
#define MODE_BLINKING_PREVIEW   8
#define MODE_DAZZLING           9
#define MODE_DAZZLING_PREVIEW   10
#define MODE_MORSE              11
#define MODE_MORSE_PREVIEW      12


// State
byte mode = 0;
unsigned long btnTime = 0;
boolean btnDown = false;
int bright, fadeDir;

// Morse Code message
char message[] = "SOS";
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
  // We just powered on!  That means either we got plugged 
  // into USB, or the user is pressing the power button.
  pinMode(DPIN_PWR,      INPUT);
  digitalWrite(DPIN_PWR, LOW);

  // Initialize GPIO
  pinMode(DPIN_RLED_SW,  INPUT);
  pinMode(DPIN_GLED,     OUTPUT);
  pinMode(DPIN_DRV_MODE, OUTPUT);
  pinMode(DPIN_DRV_EN,   OUTPUT);
  pinMode(DPIN_ACC_INT,  INPUT);
  pinMode(DPIN_PGOOD,    INPUT);
  digitalWrite(DPIN_DRV_MODE, LOW);
  digitalWrite(DPIN_DRV_EN,   LOW);
  digitalWrite(DPIN_ACC_INT,  HIGH);

  // Initialize serial busses
  Serial.begin(9600);
  Wire.begin();

  // Configure accelerometer
  byte config[] = {
    ACC_REG_INTS,  // First register (see next line)
    0xE4,  // Interrupts: shakes, taps
    0x00,  // Mode: not enabled yet
    0x00,  // Sample rate: 120 Hz
    0x0F,  // Tap threshold
    0x10   // Tap debounce samples
  };
  Wire.beginTransmission(ACC_ADDRESS);
  Wire.write(config, sizeof(config));
  Wire.endTransmission();

  // Enable accelerometer
  byte enable[] = {
    ACC_REG_MODE, 0x01      };  // Mode: active!
  Wire.beginTransmission(ACC_ADDRESS);
  Wire.write(enable, sizeof(enable));
  Wire.endTransmission();

  btnTime = millis();
  btnDown = digitalRead(DPIN_RLED_SW);
  mode = MODE_OFF;

  Serial.println("Powered up!");
}

void loop()
{
  static unsigned long lastTime, lastTempTime, lastAccTime, lastModeTime;
  static byte blink;
  unsigned long time = millis();

  // Check the state of the charge controller
  int chargeState = analogRead(APIN_CHARGE);
  if (chargeState < 128)  // Low - charging
  {
    digitalWrite(DPIN_GLED, (time&0x0100)?LOW:HIGH);
  }
  else if (chargeState > 768) // High - charged
  {
    digitalWrite(DPIN_GLED, HIGH);
  }
  else // Hi-Z - shutdown
  {
    digitalWrite(DPIN_GLED, LOW);    
  }

  // Check the serial port
  if (Serial.available())
  {
    char c = Serial.read();
    switch (c)
    {
    case 's':
      {
        int temperature = analogRead(APIN_TEMP);
        Serial.print("Temperature = ");
        Serial.println(temperature);

        char accel[3];
        readAccel(accel);
        Serial.print("Acceleration = ");
        Serial.print(accel[0], DEC);
        Serial.print(", ");
        Serial.print(accel[1], DEC);
        Serial.print(", ");
        Serial.println(accel[2], DEC);

        byte pgood = digitalRead(DPIN_PGOOD);
        Serial.print("LED driver power good = ");
        Serial.println(pgood?"Yes":"No");
      }
      break;
    }
  }

  // Check the temperature sensor
  if (time-lastTempTime > 1000)
  {
    lastTempTime = time;
    int temperature = analogRead(APIN_TEMP);
    Serial.print("Temp: ");
    Serial.println(temperature);
    if (temperature > OVERTEMP && mode != MODE_OFF)
    {
      Serial.println("Overheating!");

      for (int i = 0; i < 6; i++)
      {
        digitalWrite(DPIN_DRV_MODE, LOW);
        delay(100);
        digitalWrite(DPIN_DRV_MODE, HIGH);
        delay(100);
      }
      digitalWrite(DPIN_DRV_MODE, LOW);

      mode = MODE_LOW;
    }
  }

  // Check if the accelerometer wants to interrupt
  byte tapped = 0, shaked = 0;
  if (!digitalRead(DPIN_ACC_INT))
  {
    Wire.beginTransmission(ACC_ADDRESS);
    Wire.write(ACC_REG_TILT);
    Wire.endTransmission(false);       // End, but do not stop!
    Wire.requestFrom(ACC_ADDRESS, 1);  // This one stops.
    byte tilt = Wire.read();

    if (time-lastAccTime > 500)
    {
      lastAccTime = time;

      tapped = !!(tilt & 0x20);
      shaked = !!(tilt & 0x80);

      if (tapped) Serial.println("Tap!");
      if (shaked) Serial.println("Shake!");
    }
  }

  // Do whatever this mode does
  switch (mode)
  {
  case MODE_FADE:
    if (time-lastTime > 5)
    {
      lastTime = time;
      if (fadeDir>0 && bright==255) fadeDir = -1;
      if (fadeDir<0 && bright==0  ) fadeDir =  1;
      bright += fadeDir;
      unsigned int gamma = (bright*bright)>>8;
      if (gamma < 6) gamma = 6;
      analogWrite(DPIN_DRV_EN, gamma);
    }
    break;
  case MODE_BLINKING:
  case MODE_BLINKING_PREVIEW:
    if (time-lastTime < 250) break;
    lastTime = time;
    blink = !blink;
    digitalWrite(DPIN_DRV_EN, blink);
    break;
  case MODE_DAZZLING:
  case MODE_DAZZLING_PREVIEW:
    if (time-lastTime < 10) break;
    lastTime = time;
    digitalWrite(DPIN_DRV_EN, random(4)<1);
    break;
  case MODE_MORSE:  
  case MODE_MORSE_PREVIEW:  
    for (int i = 0; i < sizeof(message); i++)
    {
      char ch = message[i];
      if (ch == ' ')
      {
        // 7 beats between words, but 3 have already passed
        // at the end of the last character
        delay(millisPerBeat * 4);
      }
      else
      {
        // Remap ASCII to the morse table
        if      (ch >= 'A' && ch <= 'Z') ch -= 'A';
        else if (ch >= 'a' && ch <= 'z') ch -= 'a';
        else if (ch >= '0' && ch <= '9') ch -= '0' - 26;
        else continue;

        // Extract the symbols and length
        byte curChar  = morse[ch] & 0x00FF;
        byte nSymbols = morse[ch] >> 8;

        // Play each symbol
        for (int j = 0; j < nSymbols; j++)
        {
          digitalWrite(DPIN_DRV_EN, HIGH);
          if (curChar & 1)  // Dash - 3 beats
            delay(millisPerBeat * 3);
          else              // Dot - 1 beat
          delay(millisPerBeat);
          digitalWrite(DPIN_DRV_EN, LOW);
          // One beat between symbols
          delay(millisPerBeat);
          curChar >>= 1;
        }
        // 3 beats between characters, but one already
        // passed after the last symbol.
        delay(millisPerBeat * 2);
      }
    }
    // End of the message, long pause before repeat
    delay(millisPerBeat * 10);
  }

  // Check for mode changes
  byte newMode = mode;
  byte newBtnDown = digitalRead(DPIN_RLED_SW);
  switch (mode)
  {
  case MODE_OFF:
    if (btnDown && !newBtnDown)  // Button released
      newMode = MODE_LOW;
    if (btnDown && newBtnDown && (time-btnTime)>500)  // Held
      newMode = MODE_FADE;
    break;
  case MODE_LOW:
    if (btnDown && newBtnDown && (time-btnTime)>500)  // Held
      newMode = MODE_FADE;
    if (btnDown && !newBtnDown)  // Button released
      if (time-lastModeTime > 5000)
        newMode = MODE_OFF;
      else newMode = MODE_MED;
    break;
  case MODE_MED:
    if (btnDown && newBtnDown && (time-btnTime)>500)  // Held
      newMode = MODE_FADE;
    if (btnDown && !newBtnDown)  // Button released
      if (time-lastModeTime > 5000)
        newMode = MODE_OFF;
      else newMode = MODE_HIGH;
    break;
  case MODE_HIGH:
    if (btnDown && !newBtnDown)  // Button released
      newMode = MODE_OFF;
    if (btnDown && newBtnDown && (time-btnTime)>500)  // Held
      newMode = MODE_FADE;
    break;
  case MODE_FADE:
    if (!btnDown && !newBtnDown && (time-btnTime)>50)
      newMode = MODE_ON;
    if (btnDown && newBtnDown && tapped)
      newMode = MODE_BLINKING_PREVIEW;
    break;
  case MODE_ON:
    if (btnDown && !newBtnDown)  // Button released
      newMode = MODE_OFF;
    if (btnDown && newBtnDown && (time-btnTime)>500)
      newMode = MODE_FADE;
    if (btnDown && newBtnDown && tapped)
      newMode = MODE_BLINKING_PREVIEW;
    break;
  case MODE_BLINKING:
    if (btnDown && !newBtnDown)  // Button released
      newMode = MODE_OFF;
    if (btnDown && newBtnDown && (time-btnTime)>500)  // Held
      newMode = MODE_BLINKING_PREVIEW;
    break;
  case MODE_BLINKING_PREVIEW:
    if (btnDown && !newBtnDown)  // Button released
      newMode = MODE_BLINKING;
    if (btnDown && newBtnDown && tapped)
      newMode = MODE_DAZZLING_PREVIEW;
    break;
  case MODE_DAZZLING:
    if (btnDown && !newBtnDown)  // Button released
      newMode = MODE_OFF;
    if (btnDown && newBtnDown && (time-btnTime)>500)  // Held
      newMode = MODE_DAZZLING_PREVIEW;
    break;
  case MODE_DAZZLING_PREVIEW:
    if (btnDown && !newBtnDown)  // Button released
      newMode = MODE_DAZZLING;
    if (btnDown && newBtnDown && tapped)
      newMode = MODE_MORSE_PREVIEW;
    break;
  case MODE_MORSE:
    // Click to turn off don't work, need to keep pressed so back to fading!
    if (btnDown && newBtnDown && (time-btnTime)>50)  // Held
      newMode = MODE_FADE;
    break;
  case MODE_MORSE_PREVIEW:
    if (btnDown && !newBtnDown)  // Button released
      newMode = MODE_MORSE;
    if (btnDown && newBtnDown && tapped)
      newMode = MODE_FADE;
    break;
  }

  // Power down if inactive except in MODE_MORSE!
  if ((time - max(lastAccTime,lastModeTime) > 600000UL ) && newMode != MODE_MORSE && mode != MODE_OFF ) { // default 10 minutes
    newMode = MODE_OFF;
    Serial.println("Mode = off");
  }

  // Do the mode transitions
  if (newMode != mode)
  {
    lastModeTime = millis();
    switch (newMode)
    {
    case MODE_OFF:
      Serial.println("Mode = off");
      pinMode(DPIN_PWR, OUTPUT);
      digitalWrite(DPIN_PWR, LOW);
      digitalWrite(DPIN_DRV_MODE, LOW);
      digitalWrite(DPIN_DRV_EN, LOW);
      break;
    case MODE_LOW:
      Serial.println("Mode = low");
      pinMode(DPIN_PWR, OUTPUT);
      digitalWrite(DPIN_PWR, HIGH);
      digitalWrite(DPIN_DRV_MODE, LOW);
      analogWrite(DPIN_DRV_EN, 64);
      break;
    case MODE_MED:
      Serial.println("Mode = medium");
      pinMode(DPIN_PWR, OUTPUT);
      digitalWrite(DPIN_PWR, HIGH);
      digitalWrite(DPIN_DRV_MODE, LOW);
      analogWrite(DPIN_DRV_EN, 255);
      break;
    case MODE_HIGH:
      Serial.println("Mode = high");
      pinMode(DPIN_PWR, OUTPUT);
      digitalWrite(DPIN_PWR, HIGH);
      digitalWrite(DPIN_DRV_MODE, HIGH);
      analogWrite(DPIN_DRV_EN, 255);
      break;
    case MODE_FADE:
      Serial.println("Mode = fade");
      pinMode(DPIN_PWR, OUTPUT);
      digitalWrite(DPIN_PWR, HIGH);
      digitalWrite(DPIN_DRV_MODE, HIGH);
      bright = 0;
      fadeDir = 1;
      break;
    case MODE_ON:
      Serial.println("Mode = fade_fixed_on");
      pinMode(DPIN_PWR, OUTPUT);
      digitalWrite(DPIN_PWR, HIGH);
      digitalWrite(DPIN_DRV_MODE, HIGH);
      break;
    case MODE_BLINKING:
    case MODE_BLINKING_PREVIEW:
      Serial.println("Mode = blinking");
      pinMode(DPIN_PWR, OUTPUT);
      digitalWrite(DPIN_PWR, HIGH);
      digitalWrite(DPIN_DRV_MODE, LOW);
      break;
    case MODE_DAZZLING:
    case MODE_DAZZLING_PREVIEW:
      Serial.println("Mode = dazzling");
      pinMode(DPIN_PWR, OUTPUT);
      digitalWrite(DPIN_PWR, HIGH);
      digitalWrite(DPIN_DRV_MODE, HIGH);
      break;
    case MODE_MORSE:
    case MODE_MORSE_PREVIEW:
      Serial.println("Mode = Morse");
      pinMode(DPIN_PWR, OUTPUT);
      digitalWrite(DPIN_PWR, HIGH);
      digitalWrite(DPIN_DRV_MODE, HIGH);
      break;
    }

    mode = newMode;
  }

  // Remember button state so we can detect transitions
  if (newBtnDown != btnDown)
  {
    btnTime = time;
    btnDown = newBtnDown;
    delay(50);
  }
}

void readAccel(char *acc)
{
  while (1)
  {
    Wire.beginTransmission(ACC_ADDRESS);
    Wire.write(ACC_REG_XOUT);
    Wire.endTransmission(false);       // End, but do not stop!
    Wire.requestFrom(ACC_ADDRESS, 3);  // This one stops.

    for (int i = 0; i < 3; i++)
    {
      if (!Wire.available())
        continue;
      acc[i] = Wire.read();
      if (acc[i] & 0x40)  // Indicates failed read; redo!
        continue;
      if (acc[i] & 0x20)  // Sign-extend
        acc[i] |= 0xC0;
    }
    break;
  }
}

