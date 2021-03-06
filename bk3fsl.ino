/*
 * --- bk3fsl ---
 *
 * Black Knight: Sword of Rage - Flaming Speaker Lights
 *
 * Mode-changing version by Fubar.
 * Adapted from Fire2012WithPalette example code from FastLED
 * with parameters from ScottyC on pinside.
 *
 *  07 July 2021
 */
#include <FastLED.h>
#include <NeoHWSerial.h>

// Output separate effects on D4 and D5
#define LED_PIN0   4
#define LED_PIN1   5

#define COLOR_ORDER GRB
#define CHIPSET     WS2811
#define NUM_LEDS    39

#define BRIGHTNESS  200
#define FRAMES_PER_SECOND 40
#define POKE_INTERVAL_MS 1000 /* mode status update rate */
#define MODE_RUNNING_LINGER 3 /* linger when a mode stops running - catches glitches */

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

// Fire palette - changed based on running mode.
CRGBPalette16 gPal;

unsigned long last_fire = 0; // timestamp of last frame
volatile unsigned long last_rx = 0; // timestamp of last UART receive

/* Mode table we want to decode:

Mode   Lit   CanStart   Running   Spotted   Complete   Completions
------------------------------------------------------------------
   0 |      |    Y    |    Y    |        |           |       0    | (Burning Sands)
   1 |      |    Y    |         |        |           |       0    | (Mud Bog)
   2 |      |    Y    |         |        |           |       0    | (Molten Fire)
   3 |      |    Y    |         |        |           |       0    | (Wicked Cavern)
   4 |      |    Y    |         |        |           |       0    | (Deep Freeze)
   5 |      |    Y    |         |        |           |       0    | (Black Castle)
   6 |      |    Y    |         |        |           |       0    | (Catapult Multiball)
   7 |      |    Y    |         |        |           |       0    | (Triple Knight's Challenge)
   8 |      |    Y    |         |        |           |       0    | (INVALID)
   9 |      |    Y    |         |        |           |       0    | (INVALID)
  10 |      |    Y    |         |        |           |       0    | (INVALID)
  11 |      |    Y    |         |        |           |       0    | (INVALID)
  12 |      |    Y    |         |        |           |       0    | (WAR Hurry Ups)
  21 |      |    Y    |         |        |           |       0    | (Black Knight Wizard Multiball)
  22 |      |    Y    |         |        |           |       0    | (THE KINGS RANSOM)
  23 |      |    Y    |         |        |           |       0    | (SUPER CATAPULT Multiball)
  24 |      |    Y    |         |        |           |       0    | (RAGE Multiball)
  25 |      |    Y    |         |        |           |       0    | (Last Chance)
*/
#define NUM_MODES 26
int mode_running[NUM_MODES];

// Line buffer
#define BUF_SZ 256
volatile unsigned char buf[BUF_SZ];
volatile unsigned col = 0; // Set to 0 each time we see a newline.

volatile bool game_running = false;
volatile bool retro_mode_running = false; // Switches the lights off

static void uart_rx_interrupt(uint8_t c)
{
  last_rx = millis();

  if (c == '\n' || c == '\r' || col == BUF_SZ-1) {
    col = 0;
  } else {
    buf[col++] = c;
  }
  buf[col] = 0;

  if (col == 28) {
    // This is horrible but it does the job.
    if (buf[0] == ' ' && buf[1] == ' ' &&
        (buf[2] == ' ' || (buf[2] >= '0' && buf[2] <= '9')) &&
        buf[3] >= '0' && buf[3] <= '9' &&
        buf[5] == '|' && buf[12] == '|' && buf[22] == '|') {
      bool running = (buf[27] == 'Y');
      unsigned mode = buf[3] - '0';
      if (buf[2] != ' ')
        mode += 10 * (buf[2] - '0');
      if (mode < NUM_MODES) {
        game_running = true;
        if (running)
          mode_running[mode] = MODE_RUNNING_LINGER;
        else if (mode_running[mode] != 0)
          mode_running[mode]--;
        retro_mode_running = mode_running[21] || mode_running[22] || mode_running[25];
      }
    }
  }
}

void setup()
{
  // NB: if you change speed here, change the avrdude baudrate in Makefile to match.
  NeoSerial.attachInterrupt(uart_rx_interrupt);
  NeoSerial.begin(115200);

  // The first strip gets leds[0..1], the second gets leds[2..3]
  FastLED.addLeds<CHIPSET, LED_PIN0, COLOR_ORDER>(leds[0], 2 * NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<CHIPSET, LED_PIN1, COLOR_ORDER>(leds[2], 2 * NUM_LEDS).setCorrection(TypicalLEDStrip);

  FastLED.setBrightness(BRIGHTNESS);
}

CRGBPalette16 mode_palettes[] = {
  //  0  Burning sands
  [0] = CRGBPalette16(CRGB::Black, CRGB::Yellow, CRGB::SandyBrown),
  //  1  Mud bog
  [1] = CRGBPalette16(CRGB::Black, CRGB::DarkGreen, CRGB::Green),
  //  2  Molten fire
  [2] = CRGBPalette16(CRGB::Black, CRGB::Red, CRGB::Orange),
  //  3  Wicked cavern
  [3] = CRGBPalette16(CRGB::Black, CRGB::DarkRed, CRGB::Red),
  //  4  Deep freeze
  [4] = CRGBPalette16(CRGB::Black, CRGB::Cyan, CRGB::Teal),
  //  5  Black castle
  [5] = CRGBPalette16(CRGB::Black, CRGB::Purple, CRGB::White),
  //  6  Catapult multiball
  [6] = CRGBPalette16(CRGB::Black, 0xFF2000, 0xFF8000), // Orange
  //  7  3K multiball
  [7] = CRGBPalette16(CRGB::Black, CRGB::DarkBlue, CRGB::Blue),
  // 8..12  WAR hurry-up
  // 21  BK wizard
  // 22  BK2K wizard
  // 23  Super catapult MB
  // 24  RAGE MB
  // 25  Last Chance
};

void UpdatePalette()
{
  for (size_t i = 0; i < sizeof(mode_palettes) / sizeof(mode_palettes[0]); i++) {
    if (mode_running[i]) {
      gPal = mode_palettes[i];
      return;
    }
  }

  if (mode_running[23] || mode_running[24]) {
    gPal = RainbowColors_p;
    return;
  }

  // Default (use molten fire)
  gPal = mode_palettes[2];
}

void loop()
{
  if (NeoSerial) {
    if ((millis() - last_rx) > POKE_INTERVAL_MS) {
      NeoSerial.write("mode status\r\n");
    }
  }

  // If the game never starts (maybe not wired up), still display effect after
  // a while.
  if (millis() > 60000UL)
    game_running = true;

  if (millis() - last_fire > 1000 / FRAMES_PER_SECOND) {
    last_fire = millis();
    UpdatePalette();
    for (int i = 0; i < 4; i++)
      Fire2012WithPalette(i, !(i & 1)); // reverse every other segment
    // NB: FastLED will disable interrupts while this is running!
    if (game_running && !retro_mode_running)
      FastLED.show();
    else
      FastLED.clear(true);
  }
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
