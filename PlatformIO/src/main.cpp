#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <tinyNeoPixel.h>
#include <Adafruit_LIS3DH.h>
#include <AS1115.h>
#include <avr/sleep.h>
#include "OneButton.h"

#define TOTAL_LEDS 90
#define MAX_BRIGHTNESS 50
tinyNeoPixel leds = tinyNeoPixel(TOTAL_LEDS, PIN_PA1, NEO_GRB);

AS1115 as = AS1115(0x00); //!!FIXED!!  AS1115 is connected a bit weird. the order of 7 segs is bacd, not abcd

int rgb[3] = {0, 0, 0};


OneButton button1(PIN_PC1, true, true);
OneButton button2(PIN_PC2, true, true);
OneButton button3(PIN_PC3, true, true);
long button1LongPress = 0;
long button2LongPress = 0;
long button3LongPress = 0;

Adafruit_LIS3DH lis = Adafruit_LIS3DH();

int orientation = -1;
long lastAccelRead = 0;

enum State
{
  COUNTDOWN,
  TIMESET
};
State state = COUNTDOWN;

long timerLength = 15000; // milliseconds
long startTime = millis();
long prevTime = 0;
long lastFrame = 0;
long FrameTimer = 0;
int frame = 0;
bool end = true;
bool set = false;
bool skipFall = false;

#define NOTE_C5  523
#define NOTE_F5  698
#define NOTE_FS5 740
int buzzer = PIN_PA2;
int melody[] = {
  NOTE_FS5, NOTE_C5,
  NOTE_FS5, NOTE_C5,
  NOTE_FS5, NOTE_C5,
  NOTE_FS5, NOTE_C5
};

int noteDurations[] = {
  4, 4,
  4, 4,
  8, 8,
  8, 1
};

// functions
long countDown(long timerLength, long _startTime);
void timeBasedLight(long timeLeft, long totalTime);
void timeBasedColor(long timeLeft, long totalTime);
void theaterChase(uint32_t color, int wait, int iterations);
void click1();
void click2();
void click3();
void dClick1();
void dClick2();
void dClick3();
void longPress1();
void longPress2();
void longPress3();
void duringLongPress1();
void duringLongPress2();
void duringLongPress3();
long normalizeTime(long time);
void timeSet();
void asWrite(long _number);
void handleAccelEvents();
void inBetweenFrames(long timeLeft, long totalTime);

void setup()
{
  leds.begin();
  leds.setBrightness(MAX_BRIGHTNESS);
  if (!lis.begin(0x18))
  { // change this to 0x19 for alternative i2c address
    while (1)
      yield();
  }
  lis.setRange(LIS3DH_RANGE_2_G);
  lis.setDataRate(LIS3DH_DATARATE_400_HZ);
  delay(100);

  as.init(4, 6);
  delay(100); // give sensor time to stabilize
  as.clear();


  button1.attachClick(click1);
  button2.attachClick(click2);
  button3.attachClick(click3);
  button1.attachLongPressStart(longPress1);
  button2.attachLongPressStart(longPress2);
  button3.attachLongPressStart(longPress3);
  button1.attachDuringLongPress(duringLongPress1);
  button2.attachDuringLongPress(duringLongPress2);
  button3.attachDuringLongPress(duringLongPress3);

  button1.setDebounceTicks(50);
  button2.setDebounceTicks(50);
  button3.setDebounceTicks(50);

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
}

void loop()
{
  button1.tick();
  button2.tick();
  button3.tick();

  handleAccelEvents();

  if (end)
  {
    return;
  }
  long timeLeft = countDown(timerLength, startTime);
  if (timeLeft / 1000 != prevTime)
  {
    lastFrame = millis();
    frame = 0;
    prevTime = timeLeft / 1000;
    skipFall = true;
  }
  // call this function every 50ms
  if (millis() - FrameTimer > 50 && !skipFall)
  {
    FrameTimer = millis();
    inBetweenFrames(timeLeft, timerLength);
  }
  skipFall = false;
  // we need to rearrange the digits to be in the right order

  asWrite(normalizeTime(timeLeft));
  if (timeLeft <= 0)
  {
    theaterChase(leds.Color(255, 0, 0), 50, 50);
    // set all leds to black
    for (int i = 0; i < TOTAL_LEDS; i++)
    {
      leds.setPixelColor(i, 0, 0, 0);
    }
    leds.show();
    // clear the 7 segs
    as.clear();
    end = true;
  }
}

void handleAccelEvents()
{
  if (millis() - lastAccelRead < 100)
  { // read accel every 100ms
    return;
  }
  lastAccelRead = millis();
  lis.read();
  // sensors_event_t event;
  // lis.getEvent(&event);
  if (lis.z > 10000 && orientation != 0)
  {
    orientation = 0;
    startTime = millis();
  }
  else if (lis.z < -10000 && orientation != 1)
  {
    orientation = 1;
    startTime = millis();
  }
}

long normalizeTime(long time)
{
  long digit1 = (time / 1000) % 10;
  long digit2 = ((time / 1000) / 10) % 10; // the 3rd and 4th digits are a repeat of the first two, but reversed
  time = digit2 * 10 + digit1;
  return time;
}

void click1()
{
  switch (state)
  {
  case COUNTDOWN:
    startTime = millis();
    end = false;
    break;
  case TIMESET:
    timerLength += 1000;
    if (timerLength > 99000)
    {
      timerLength = 99000;
    }
    break;
  }
}

void click2()
{
  switch (state)
  {
  case COUNTDOWN:
    startTime = millis();
    end = false;
    break;
  case TIMESET:
    timerLength -= 1000;
    if (timerLength < 1000)
    {
      timerLength = 1000;
    }
    break;
  }
}

void click3()
{
  switch (state)
  {
  case COUNTDOWN:
    startTime = millis();
    end = false;
    break;
  case TIMESET:
    set = true;
    end = true;
    break;
  }
}


void longPress1()
{
  switch (state)
  {
  case COUNTDOWN:
    state = TIMESET;
    timeSet();
    break;
  case TIMESET:
    break;
  }
}

void longPress2()
{
  switch (state)
  {
  case COUNTDOWN:
    state = TIMESET;
    timeSet();
    break;
  case TIMESET:
    break;
  }
}

void longPress3()
{
  switch (state)
  {
  case COUNTDOWN:
    state = TIMESET;
    timeSet();
    break;
  case TIMESET:
    break;
  }
}

void duringLongPress1()
{
  switch (state)
  {
  case COUNTDOWN:
    break;
  case TIMESET:
    if (millis() - button1LongPress > 1000)
    {
      timerLength += 10000;
      if (timerLength > 99000)
      {
        timerLength = 99000;
      }
      button1LongPress = millis();
    }
    break;
  }
}

void duringLongPress2()
{
  switch (state)
  {
  case COUNTDOWN:
    break;
  case TIMESET:
    if (millis() - button2LongPress > 1000)
    {
      timerLength -= 10000;
      if (timerLength < 1000)
      {
        timerLength = 1000;
      }
      button2LongPress = millis();
    }
    break;
  }
}

void duringLongPress3()
{
}

void timeSet()
{
  // clear the neo pixels
  leds.clear();
  leds.show();
  set = false;
  // set as to blink
  long lastBlink = millis();
  while (!set)
  {
    if (millis() - lastBlink > 500)
    {
      as.clear();
      lastBlink = millis();
    }
    if (millis() - lastBlink > 250)
    {
      asWrite(normalizeTime(timerLength));
    }
    button1.tick();
    button2.tick();
    button3.tick();
    handleAccelEvents();
    if (!button1.isLongPressed())
    {
      button1LongPress = 0;
    }
    if (!button2.isLongPressed())
    {
      button2LongPress = 0;
    }
    if (!button3.isLongPressed())
    {
      button3LongPress = 0;
    }
  }
  state = COUNTDOWN;
  startTime = millis();
  end = false;
}

void timeBasedLight(long timeLeft, long totalTime)
{
  // 15 leds on top, 15 on bottom
  int numLedsOn = (int)(((float)timeLeft / totalTime) * 15);
  timeBasedColor(timeLeft, totalTime);
  leds.clear();
  if (orientation == 0)
  {
    for (int i = 0; i < numLedsOn; i++)
    {
      leds.setPixelColor(i, rgb[0], rgb[1], rgb[2]);
      leds.setPixelColor(45 + i, rgb[0], rgb[1], rgb[2]);
      leds.setPixelColor(60 + i, rgb[0], rgb[1], rgb[2]);
    }
    for (int i = 0; i < 15 - numLedsOn; i++)
    {
      leds.setPixelColor(29 - i, rgb[0], rgb[1], rgb[2]);
      leds.setPixelColor(44 - i, rgb[0], rgb[1], rgb[2]);
      leds.setPixelColor(89 - i, rgb[0], rgb[1], rgb[2]);
    }
  }
  else
  { // if orientation is 1
    for (int i = 0; i < numLedsOn; i++)
    {
      leds.setPixelColor(29 - i, rgb[0], rgb[1], rgb[2]);
      leds.setPixelColor(44 - i, rgb[0], rgb[1], rgb[2]);
      leds.setPixelColor(89 - i, rgb[0], rgb[1], rgb[2]);
    }
    for (int i = 0; i < 15 - numLedsOn; i++)
    {
      leds.setPixelColor(14 - i, rgb[0], rgb[1], rgb[2]);
      leds.setPixelColor(59 - i, rgb[0], rgb[1], rgb[2]);
      leds.setPixelColor(74 - i, rgb[0], rgb[1], rgb[2]);
    }
  }
  lastFrame = millis();
  frame = 0;
  leds.show();
}

void inBetweenFrames(long timeLeft, long totalTime)
{ // adds the sand falling effect. will be called every 50 ms
  int numLedsOn = (int)(((float)timeLeft / totalTime) * 15);
  numLedsOn--; // to make it so that the most recent sand is falling
  timeBasedColor(timeLeft, totalTime);
  leds.clear();
  int updateFrame; // number of ms between each frame
  if (15 - numLedsOn + 1 < 6)
  {
    updateFrame = 200;
  }
  else if (15 - numLedsOn < 10)
  {
    updateFrame = 250;
  }
  else if (15 - numLedsOn < 13)
  {
    updateFrame = 300;
  }
  else if (15 - numLedsOn < 15)
  {
    updateFrame = 500;
  }
  else
  {
    updateFrame = 1000;
  }

  int fallingLed;
  // now to add the sand falling effect
  if (millis() - lastFrame > updateFrame)
  {
    lastFrame = millis();

    switch (frame)
    {
    case 0:
      fallingLed = 0;
      break;
    case 1:
      fallingLed = 2;
      break;
    case 2:
      fallingLed = 4;
      break;
    case 3:
      fallingLed = 7;
      break;
    default:
      fallingLed = 1;
      break;
    }
    frame++;
  }

  if (orientation == 0)
  {
    for (int i = 0; i < numLedsOn; i++)
    {
      leds.setPixelColor(i, rgb[0], rgb[1], rgb[2]);
      leds.setPixelColor(45 + i, rgb[0], rgb[1], rgb[2]);
      leds.setPixelColor(60 + i, rgb[0], rgb[1], rgb[2]);
    }
    for (int i = 0; i < 15 - numLedsOn; i++)
    {
      leds.setPixelColor(29 - i, rgb[0], rgb[1], rgb[2]);
      leds.setPixelColor(44 - i, rgb[0], rgb[1], rgb[2]);
      leds.setPixelColor(89 - i, rgb[0], rgb[1], rgb[2]);
    }
    // now to add the sand falling effect
    if (numLedsOn > 2)
    {
      leds.setPixelColor(15 + fallingLed, rgb[0], rgb[1], rgb[2]);
      leds.setPixelColor(30 + fallingLed, rgb[0], rgb[1], rgb[2]);
      leds.setPixelColor(75 + fallingLed, rgb[0], rgb[1], rgb[2]);
    }
  }
  else
  { // if orientation is 1
    for (int i = 0; i < numLedsOn; i++)
    {
      leds.setPixelColor(29 - i, rgb[0], rgb[1], rgb[2]);
      leds.setPixelColor(44 - i, rgb[0], rgb[1], rgb[2]);
      leds.setPixelColor(89 - i, rgb[0], rgb[1], rgb[2]);
    }
    for (int i = 0; i < 15 - numLedsOn; i++)
    {
      leds.setPixelColor(14 - i, rgb[0], rgb[1], rgb[2]);
      leds.setPixelColor(59 - i, rgb[0], rgb[1], rgb[2]);
      leds.setPixelColor(74 - i, rgb[0], rgb[1], rgb[2]);
    }
    // now to add the sand falling effect
    if (numLedsOn > 2)
    {
      leds.setPixelColor(fallingLed, rgb[0], rgb[1], rgb[2]);
      leds.setPixelColor(45 + fallingLed, rgb[0], rgb[1], rgb[2]);
      leds.setPixelColor(60 + fallingLed, rgb[0], rgb[1], rgb[2]);
    }
  }

  leds.show();
}

long countDown(long timerLength, long _startTime)
{ // returns number of milliseconds left
  long timeLeft = timerLength - (millis() - _startTime);
  if (timeLeft < 0)
  {
    timeLeft = 0;
  }
  return timeLeft;
}

// color based on time. color will go from green to red as time goes down
void timeBasedColor(long timeLeft, long totalTime)
{
  rgb[1] = (int)(((float)timeLeft / totalTime) * 255);
  rgb[0] = 255 - rgb[1];
  rgb[2] = 0;
}

void theaterChase(uint32_t color, int wait, int iterations)
{
  long toneCounter = 0;
  int tonePos = 0;
  for (int a = 0; a < iterations; a++)
  { // Repeat 10 times...
    for (int b = 0; b < 3; b++)
    {               //  'b' counts from 0 to 2...
      leds.clear(); //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for (int c = b; c < leds.numPixels(); c += 3)
      {
        leds.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      leds.show(); // Update strip with new contents
      delay(wait); // Pause for a moment
      //tone timing using millis
      if (millis() - toneCounter > 100)
      {
        toneCounter = millis();
        tonePos++;
        if (tonePos > 2)
        {
          tonePos = 0;
        }
        tone(buzzer, melody[tonePos]);
      }
    }
  }
  noTone(buzzer);
}

void asWrite(long _number)
{
  char __number[4] = {'    '};
  sprintf(__number, "%02d", _number);
  char temp;
  if (orientation == 0)
  {
    // temp = __number[0];
    //__number[0] = __number[1];
    //__number[1] = temp;
    as.display(__number);
  }
  else
  {
    temp = __number[2];
    __number[2] = __number[0];
    __number[3] = __number[1];
    __number[0] = ' ';
    __number[1] = ' ';
    as.display(__number);
  }
  //
}