// This is necessary if you're using Atom/platform.io
#include "StandardCplusplus.h"

// Include the Tween library for smooth transitions between on and off
#include "Tween.h"


#define LAMP_FORCE_PIN 5
#define LAMP_SITH_PIN 6
#define LED_TYPE    NEOPIXEL
#define COLOR_ORDER GRB
#define MAX_BRIGHTNESS     255

#define FRAMES_PER_SECOND  120

// Look here to see how all the different FastLED patterns work...
// and add your own!
#include "patterns.h"

#define PIN_PIR 3
//#define PIN_PHOTORESISTOR 0

#define IDLE_DELAY_MILLIS 1000 * 20 // time light stays on, regardless of motion
#define IDLE_TIMEOUT_MILLIS 10000 // time we can stay in Idle mode without motion
#define FADE_UP_TIME_MILLIS 10000 // time light fades up to max
#define FADE_DOWN_TIME_MILLIS 1000 * 20 // time light slowly fades to black

unsigned long activeMillis;
unsigned long idleMillis;

enum State {
  STATE_INIT,
  STATE_OFF,
  STATE_IDLE,
  STATE_ACTIVE,
  STATE_FADING_DOWN,
  STATE_FADING_UP
} state;


// Lower numbers mean dark mode is darker
#define BRIGHTNESS_THRESHOLD 250

enum Mode {
  MODE_NIGHT,
  MODE_DAY
} lightMode;

// The Tween controls transitions between two numbers - it calculates
// the "in-between" values between the start point and the end point.
// In this case, we're using it to fade up and fade down the brightness
// when motion is detected, or when it idles out.
Tween brightnessTween;
void updateTweens(){
  if (brightnessTween.isRunning()){
    brightnessTween.update();
    FastLED.setBrightness(brightnessTween.value);
  }
}



// Handles any initialization code that needs to be run once as we start a new
// state, by considering the old state when necessary. For more on what each
// state does, look to the comments for the loop() function below.
void transitionState(State newState){
  State oldState = state;

  switch (newState){
    case STATE_INIT :
      transitionState(STATE_OFF);
      break;

    case STATE_OFF :
      FastLED.setBrightness(0);
      brightnessTween.value = 0;
      break;

    case STATE_FADING_UP :
      nextPattern();
      brightnessTween.setup(FADE_UP_TIME_MILLIS, brightnessTween.value, MAX_BRIGHTNESS, Easing::LinearEaseInOut);
      brightnessTween.play();
      break;

    case STATE_ACTIVE :
      FastLED.setBrightness(MAX_BRIGHTNESS);
      activeMillis = millis();
      break;

    case STATE_IDLE :
      idleMillis = millis();
      break;

    case STATE_FADING_DOWN :
      brightnessTween.setup(FADE_DOWN_TIME_MILLIS, brightnessTween.value, 0, Easing::LinearEaseInOut);
      brightnessTween.play();
      break;

  }

  state = newState;

  // If you have your light connected to your computer using USB, you can open
  // up a Serial Monitor window and you can observe these state transitions
  // as they occur. This can be helpful in debugging motion sensor issues.
  Serial.print(oldState);
  Serial.print(" --> ");
  Serial.println(newState);
}

void setup() {
  delay(1000); // 1 second delay for recovery. I have no idea why this is useful,
               // but it's used in every FastLED demo sketch, so it persists here!

  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,LAMP_FORCE_PIN>(leds_f, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE,LAMP_SITH_PIN>(leds_s, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(MAX_BRIGHTNESS);

  // Setup the HCS501 Personal Infra-Red motion sensor
  pinMode(PIN_PIR, INPUT);

  // Default the state to INIT
  state = STATE_INIT;
  // And let's assume Night mode.
  lightMode = MODE_NIGHT;

  // Start output logging. Make sure you set your serial monitor's
  // baud rate to 115200 in order to not get garbled text.
  Serial.begin(115200);
}





/*  The following chart represents the possible States and their transitions:

    STATE_OFF (PIR) --> STATE_FADING_UP
    STATE_FADING_UP (anim) --> STATE_ACTIVE
    STATE_ACTIVE (timer) --> STATE_IDLE
    STATE_IDLE (PIR) --> STATE_ACTIVE
    STATE_IDLE (timer) --> STATE_FADING_DOWN
    STATE_FADING_DOWN (PIR) --> STATE_FADING_UP
    STATE_FADING_DOWN (anim) --> STATE_OFF
*/


void loop() {
  //Serial.println(analogRead(PIN_PHOTORESISTOR));
  //Serial.println(digitalRead(PIN_PIR));

  /*  We want to only check the brightness mode when we're re-activating
      after it's been off. We don't want to be switching modes in the middle
      of anything.

      The objective really is to have it behave differently at night than
      during the day, not switch constantly between different behaviours
  */

  switch(state){
    // The init state is just there to force us to transition to OFF
    case STATE_INIT :
      transitionState(STATE_OFF);
      break;


    case STATE_OFF :
      if (digitalRead(PIN_PIR)){
        transitionState(STATE_FADING_UP);
      }
      break;

    case STATE_FADING_UP :
      if (brightnessTween.isFinished()){
        transitionState(STATE_ACTIVE);
      }
      break;

    case STATE_ACTIVE :
      if (activeMillis + IDLE_DELAY_MILLIS < millis()){
        transitionState(STATE_IDLE);
      }
      break;

    case STATE_IDLE :
      if (digitalRead(PIN_PIR)){
        transitionState(STATE_ACTIVE);
      } else if (idleMillis + IDLE_TIMEOUT_MILLIS < millis()){
        transitionState(STATE_FADING_DOWN);
      }
      break;

    case STATE_FADING_DOWN :
      if (digitalRead(PIN_PIR)){
        transitionState(STATE_FADING_UP);
      } else if (brightnessTween.isFinished()){
        transitionState(STATE_OFF);
      }
      break;
  }

  updateTweens();

  // Call the current pattern function once, updating the 'leds' array
  // (see patterns.h)
  gPatterns[gCurrentPatternNumber]();
  // send the 'leds' array out to the actual LED strip
  FastLED.show();
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND);

  // do some periodic updates
  EVERY_N_MILLISECONDS( 50 ) { gHue++; } // slowly cycle the "base color" through the rainbow
}
