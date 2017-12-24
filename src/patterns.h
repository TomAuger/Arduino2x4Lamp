// Include the most excellent FastLED library for manipulating the string
// of LEDs
#include <FastLED.h>

#define NUM_LEDS    4

// Define some FaseLED patterns
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
                                   // based on the lazy way I'm getting the next pattern,
                                   // the first pattern displayed on power up
                                   // will actually be the second pattern in
                                   // the list (index 1).
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

// These two arrays represent the two 4-pixel strips on each bulb.
CRGB leds_f[NUM_LEDS];
CRGB leds_s[NUM_LEDS];



void rainbow() {
  // FastLED's built-in rainbow generator
  fill_rainbow( leds_f, NUM_LEDS, gHue, 12);
  fill_rainbow( leds_s, NUM_LEDS, (gHue + 36) % 256, 12);
}

void star_wars() {
  fill_rainbow(leds_f, NUM_LEDS, beatsin8(20, 140, 170));
  fill_rainbow(leds_s, NUM_LEDS, beatsin8(30, 0, 25) - 15 % 256);
}

void one_eighty() {
  // FastLED's built-in rainbow generator
  fill_rainbow( leds_f, NUM_LEDS, gHue, 7);
  fill_rainbow( leds_s, NUM_LEDS, (gHue + 128) % 256, 7);
}



// List of patterns to choose from.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, star_wars, one_eighty };







// This is a quick macro to overcome a shortcoming of C
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern(){
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns);
}
