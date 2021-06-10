#include <FastLED.h>

// Output separate effects on D4 and D5
#define LED_PIN0   4
#define LED_PIN1   5

#define COLOR_ORDER GRB
#define CHIPSET     WS2811
#define NUM_LEDS    39

#define BRIGHTNESS  200
#define FRAMES_PER_SECOND 40

/* COOLING: How much does the air cool as it rises?
 * Less cooling = taller flames.  More cooling = shorter flames.
 * Default 55, suggested range 20-100 */
#define COOLING  55

/* SPARKING: What chance (out of 255) is there that a new spark will be lit?
 * Higher chance = more roaring fire.  Lower chance = more flickery fire.
 * Default 120, suggested range 50-200. */
#define SPARKING 100

/* We are processing 4 segments, which form two U shapes:
 *
 *    1    0`------. .--------'2    3
 *    1    0        Y          2    3
 *    1    0        |          2    3
 *    111000      leds[]       222333
 *
 * Each segment has NUM_LEDS.
 */
CRGB leds[4][NUM_LEDS];

// Segments 0 and 2 will be reversed.
void Fire2012WithPalette(unsigned leds_idx, bool reverse);

CRGBPalette16 gPal;

void setup() {
  // The first strip gets leds[0..1], the second gets leds[2..3]
  FastLED.addLeds<CHIPSET, LED_PIN0, COLOR_ORDER>(leds[0], 2 * NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<CHIPSET, LED_PIN1, COLOR_ORDER>(leds[2], 2 * NUM_LEDS).setCorrection(TypicalLEDStrip);

  FastLED.setBrightness(BRIGHTNESS);

  // Black Knight colour palette. Other examples below...
  gPal = CRGBPalette16(CRGB::Black, CRGB::Red, CRGB::Orange);

  /* This first palette is the basic 'black body radiation' colors,
  // which run from black to red to bright yellow to white.
  //gPal = HeatColors_p;

  // First, a gradient from black to red to yellow to white -- similar to HeatColors_p
  //   gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::Yellow, CRGB::White);

  // Second, this palette is like the heat colors, but blue/aqua instead of red/yellow
  //   gPal = CRGBPalette16( CRGB::Black, CRGB::Blue, CRGB::Aqua,  CRGB::White);

  // Third, here's a simpler, three-step gradient, from black to red to white
  //   gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::White);

  // Fourth, the most sophisticated: this one sets up a new palette every
  // time through the loop, based on a hue that changes every time.
  // The palette is a gradient from black, to a dark color based on the hue,
  // to a light color based on the hue, to white. (Put the code in loop().)

  // static uint8_t hue = 0;
  // hue++;
  // CRGB darkcolor  = CHSV(hue,255,192); // pure hue, three-quarters brightness
  // CRGB lightcolor = CHSV(hue,128,255); // half 'whitened', full brightness
  // gPal = CRGBPalette16( CRGB::Black, darkcolor, lightcolor, CRGB::White);
   */
}

void loop()
{
  for (int i = 0; i < 4; i++)
    Fire2012WithPalette(i, !(i & 1)); // reverse every other segment

  FastLED.show(); // display this frame
  FastLED.delay(1000 / FRAMES_PER_SECOND);
}


/* Fire2012 with programmable Color Palette
 *
 * This code is the same fire simulation as the original "Fire2012",
 * but each heat cell's temperature is translated to color through a FastLED
 * programmable color palette, instead of through the "HeatColor(...)" function.
 *
 * Four different static color palettes are provided here, plus one dynamic one.
 *
 * The three static ones are:
 *   1. the FastLED built-in HeatColors_p -- this is the default, and it looks
 *      pretty much exactly like the original Fire2012.
 *
 *  To use any of the other palettes below, just "uncomment" the corresponding code.
 *
 *   2. a gradient from black to red to yellow to white, which is
 *      visually similar to the HeatColors_p, and helps to illustrate
 *      what the 'heat colors' palette is actually doing,
 *   3. a similar gradient, but in blue colors rather than red ones,
 *      i.e. from black to blue to aqua to white, which results in
 *      an "icy blue" fire effect,
 *   4. a simplified three-step gradient, from black to red to white, just to show
 *      that these gradients need not have four components; two or
 *      three are possible, too, even if they don't look quite as nice for fire.
 *
 * The dynamic palette shows how you can change the basic 'hue' of the
 * color palette every time through the loop, producing "rainbow fire".
 */

/* Fire2012 by Mark Kriegsman, July 2012
 * as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
 *
 * This basic one-dimensional 'fire' simulation works roughly as follows:
 * There's a underlying array of 'heat' cells, that model the temperature
 * at each point along the line.  Every cycle through the simulation,
 * four steps are performed:
 *  1) All cells cool down a little bit, losing heat to the air
 *  2) The heat from each cell drifts 'up' and diffuses a little
 *  3) Sometimes randomly new 'sparks' of heat are added at the bottom
 *  4) The heat from each cell is rendered as a color into the leds array
 *     The heat-to-color mapping uses a black-body radiation approximation.
 *
 * Temperature is in arbitrary units from 0 (cold black) to 255 (white hot).
 *
 * This simulation scales it self a bit depending on NUM_LEDS; it should look
 * "OK" on anywhere from 20 to 100 LEDs without too much tweaking.
 *
 * I recommend running this simulation at anywhere from 30-100 frames per second,
 * meaning an interframe delay of about 10-35 milliseconds.
 *
 * Looks best on a high-density LED setup (60+ pixels/meter).
 *
 *
 * There are two main parameters you can play with to control the look and
 * feel of your fire: COOLING (used in step 1 above), and SPARKING (used
 * in step 3 above).
 */


void Fire2012WithPalette(unsigned leds_idx, bool reverse)
{
  // Array of temperature readings at each simulation cell
  static uint8_t heats[4][NUM_LEDS];
  uint8_t* heat = heats[leds_idx];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      uint8_t colorindex = scale8( heat[j], 240);
      CRGB color = ColorFromPalette( gPal, colorindex);
      int pixelnumber;
      if( reverse ) {
        pixelnumber = (NUM_LEDS-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[leds_idx][pixelnumber] = color;
    }
}
