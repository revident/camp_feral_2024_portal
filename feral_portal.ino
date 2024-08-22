/* RFID Setup */
#include <Wire.h>
#include <SparkFun_Qwiic_Rfid.h>

#define RFID_ADDR 0x13 // Default I2C address 

Qwiic_Rfid myRfid(RFID_ADDR);

// Interrupt Pin on pin 6. 
// const int intPin = 6; // not used, polling instead
String tag;
const long rfidPollIntvl = 500; //milliseconds
long nextRfidPoll = 0;

const long globalDelay = 20; //milliseconds

/* DMX Setup */
#include <DmxOutput.h>
#include <DmxOutput.pio.h>

// Declare an instance of the DMX Output
// DmxOutput dmx;

//#define UNIVERSE_LENGTH 68 // 4 + 32 + 32 RGBW channels
//uint8_t universe[UNIVERSE_LENGTH + 1];

// Hacked up FastLED for PicoDMX! 3.7.3-revident
#include <FastLED.h>
//#include <dmx.h>
#define NUM_LEDS 17 // 68/4 = 17
CRGB leds[NUM_LEDS];

// DMX Channel Rotation Test
uint32_t activeScene = 0; // DMX Scene Selection via FIFO 
uint32_t propScene = 0;

// DMX TX Pin
const uint8_t dmxPin = 16;

// Heart Beat LED
const int ledPin = 25;

void setup() {
  delay(3000); // Grace for PC enumration of USB Serial device 
  Serial.begin(115200);
  Wire.begin();

  if(myRfid.begin())
    Serial.println("Ready to seek key objects..."); 
  else
    Serial.println("Could not communicate with the Qwiic RFID Reader!!!"); 

  // Put the interrupt pin in a known HIGH state. 
  // pinMode(intPin, INPUT_PULLUP); 

  // Want to clear tags sitting on the Qwiic RFID card?
  myRfid.clearTags();


  pinMode(25, OUTPUT); // heartbeat LED
}

void loop() {

  digitalWrite(ledPin, HIGH); // start heartbeat LED

  if (millis() >= nextRfidPoll) {
    nextRfidPoll = millis() + rfidPollIntvl;
    tag = myRfid.getTag();
    Serial.println(tag);

    unsigned long long int tagNum = std::strtoll(tag.c_str(), nullptr, 10);

/*
1809315414390
1320619555118
1806514646239
33010926188234
6901725320020
*/

    switch (tagNum) {
      case 408416246182 : // Black Out Test Card
        setScene(0);
        break;
      case 601411383637 : // Rainbow Test Card
        setScene(1);
        break;
      case 31337 : // narrative trigger
        setScene(3);
        break;
      case 60141224218177 : // Narrative Testing Card
        setScene(3);
        break;
      case 6901725320020 : // Narrative Testing Card
        setScene(3);
        break;
      case 601411495739 :
        setScene(4);
        break;
    }
  }


// https://arduino-pico.readthedocs.io/en/latest/multicore.html
// push DMX scene changes to fifo

  digitalWrite(ledPin, LOW); // stop heartbeat LED
}
void setup1() {
  FastLED.addLeds<DMXPICO, dmxPin, RGB>(leds, NUM_LEDS);
}

void loop1() {
  // read DMX scene changes from FIFO
  if (rp2040.fifo.available()) {
    rp2040.fifo.pop_nb(&activeScene);
    Serial.print("Switching to DMX scene ");
    Serial.println(activeScene);
  }

  switch (activeScene) {
    case 1 : // Rainbow Test Scene
      pride();
      break;
    case 2 : // Unused
      sceneSolidRed();
      break;
    case 3 : // Main Feral Scene
      scenePacifica();
      break;
    case 4 : // Test Scene
      sceneEndCheck();
      break;
    default : // Black Out
      sceneBlackOut();
  }
}

void setScene(uint32_t _sceneNum) {
  rp2040.fifo.push(_sceneNum);
}

void fadeall() { for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(250); } }

void rainbowCylon() {
  static uint8_t hue = 0;
  // First slide the led in one direction
  for(int i = 0; i < NUM_LEDS; i++) {
      // Set the i'th led to red
      leds[i] = CHSV(hue++, 255, 255);
      // Show the leds
      FastLED.show();
      // now that we've shown the leds, reset the i'th led to black
      leds[i] = CRGB::Black;
      //fadeall();
      // Wait a little bit before we loop around and do it again
      delay(100);
  }

  // Now go in the other direction.
  for(int i = (NUM_LEDS)-1; i >= 0; i--) {
    // Set the i'th led to red
    leds[i] = CHSV(hue++, 255, 255);
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    leds[i] = CRGB::Black;
    //fadeall();
    // Wait a little bit before we loop around and do it again
    delay(100);
  }
}

void sceneBlackOut() {
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
  delay(globalDelay);
}

void sceneSolidRed() {
  /* Take a number representing Red[1], Green[2], Blue[3] or White[4] Channels
  Set all channels of the color to half brightness */
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(255, 0, 0);
    // Show the leds
    FastLED.show();
    delay(100);
  }
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}

// This function draws rainbows with an ever-changing,
// widely-varying set of parameters.
void pride()
{
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;

  uint8_t sat8 = beatsin88( 87, 220, 250);
  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 1, 3000);

  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;
  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88( 400, 5,9);
  uint16_t brightnesstheta16 = sPseudotime;

  for( uint16_t i = 0 ; i < NUM_LEDS; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);

    CRGB newcolor = CHSV( hue8, sat8, bri8);

    uint16_t pixelnumber = i;
    pixelnumber = (NUM_LEDS-1) - pixelnumber;

    nblend( leds[pixelnumber], newcolor, 64);
  }

  FastLED.show();
  delay(globalDelay);
}

void sceneEndCheck() {
  leds[16] = CRGB(255, 0, 0);
  FastLED.show();
  delay(250);

  leds[16] = CRGB(0, 255, 0);
  FastLED.show();
  delay(250);

  leds[16] = CRGB(0, 0, 255);
  FastLED.show();
  delay(250);

  leds[16] = CRGB::Black;
  FastLED.show();
  delay(250);
}

void scenePacifica() {
    pacifica_loop();
    FastLED.show();
    delay(globalDelay);
}

//
// The code for this animation is more complicated than other examples, and 
// while it is "ready to run", and documented in general, it is probably not 
// the best starting point for learning.  Nevertheless, it does illustrate some
// useful techniques.
//
//
// In this animation, there are four "layers" of waves of light.  
//
// Each layer moves independently, and each is scaled separately.
//
// All four wave layers are added together on top of each other, and then 
// another filter is applied that adds "whitecaps" of brightness where the 
// waves line up with each other more.  Finally, another pass is taken
// over the led array to 'deepen' (dim) the blues and greens.
//
// The speed and scale and motion each layer varies slowly within independent 
// hand-chosen ranges, which is why the code has a lot of low-speed 'beatsin8' functions
// with a lot of oddly specific numeric ranges.
//
// These three custom blue-green color palettes were inspired by the colors found in
// the waters off the southern coast of California, https://goo.gl/maps/QQgd97jjHesHZVxQ7
//
CRGBPalette16 pacifica_palette_1 = 
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117, 
      0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x14554B, 0x28AA50 };
CRGBPalette16 pacifica_palette_2 = 
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117, 
      0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x0C5F52, 0x19BE5F };
CRGBPalette16 pacifica_palette_3 = 
    { 0x000208, 0x00030E, 0x000514, 0x00061A, 0x000820, 0x000927, 0x000B2D, 0x000C33, 
      0x000E39, 0x001040, 0x001450, 0x001860, 0x001C70, 0x002080, 0x1040BF, 0x2060FF };
 
 
void pacifica_loop()
{
  // Increment the four "color index start" counters, one for each wave layer.
  // Each is incremented at a different speed, and the speeds vary over time.
  static uint16_t sCIStart1, sCIStart2, sCIStart3, sCIStart4;
  static uint32_t sLastms = 0;
  uint32_t ms = GET_MILLIS();
  uint32_t deltams = ms - sLastms;
  sLastms = ms;
  uint16_t speedfactor1 = beatsin16(3, 179, 269);
  uint16_t speedfactor2 = beatsin16(4, 179, 269);
  uint32_t deltams1 = (deltams * speedfactor1) / 256;
  uint32_t deltams2 = (deltams * speedfactor2) / 256;
  uint32_t deltams21 = (deltams1 + deltams2) / 2;
  sCIStart1 += (deltams1 * beatsin88(1011,10,13));
  sCIStart2 -= (deltams21 * beatsin88(777,8,11));
  sCIStart3 -= (deltams1 * beatsin88(501,5,7));
  sCIStart4 -= (deltams2 * beatsin88(257,4,6));
 
  // Clear out the LED array to a dim background blue-green
  fill_solid( leds, NUM_LEDS, CRGB( 2, 6, 10));
 
  // Render each of four layers, with different scales and speeds, that vary over time
  pacifica_one_layer( pacifica_palette_1, sCIStart1, beatsin16( 3, 11 * 256, 14 * 256), beatsin8( 10, 70, 130), 0-beat16( 301) );
  pacifica_one_layer( pacifica_palette_2, sCIStart2, beatsin16( 4,  6 * 256,  9 * 256), beatsin8( 17, 40,  80), beat16( 401) );
  pacifica_one_layer( pacifica_palette_3, sCIStart3, 6 * 256, beatsin8( 9, 10,38), 0-beat16(503));
  pacifica_one_layer( pacifica_palette_3, sCIStart4, 5 * 256, beatsin8( 8, 10,28), beat16(601));
 
  // Add brighter 'whitecaps' where the waves lines up more
  pacifica_add_whitecaps();
 
  // Deepen the blues and greens a bit
  pacifica_deepen_colors();
}
 
// Add one layer of waves into the led array
void pacifica_one_layer( CRGBPalette16& p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff)
{
  uint16_t ci = cistart;
  uint16_t waveangle = ioff;
  uint16_t wavescale_half = (wavescale / 2) + 20;
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    waveangle += 250;
    uint16_t s16 = sin16( waveangle ) + 32768;
    uint16_t cs = scale16( s16 , wavescale_half ) + wavescale_half;
    ci += cs;
    uint16_t sindex16 = sin16( ci) + 32768;
    uint8_t sindex8 = scale16( sindex16, 240);
    CRGB c = ColorFromPalette( p, sindex8, bri, LINEARBLEND);
    leds[i] += c;
  }
}
 
// Add extra 'white' to areas where the four layers of light have lined up brightly
void pacifica_add_whitecaps()
{
  uint8_t basethreshold = beatsin8( 9, 55, 65);
  uint8_t wave = beat8( 7 );
  
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    uint8_t threshold = scale8( sin8( wave), 20) + basethreshold;
    wave += 7;
    uint8_t l = leds[i].getAverageLight();
    if( l > threshold) {
      uint8_t overage = l - threshold;
      uint8_t overage2 = qadd8( overage, overage);
      leds[i] += CRGB( overage, overage2, qadd8( overage2, overage2));
    }
  }
}
 
// Deepen the blues and greens
void pacifica_deepen_colors()
{
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    leds[i].blue = scale8( leds[i].blue,  145); 
    leds[i].green= scale8( leds[i].green, 200); 
    leds[i] |= CRGB( 2, 5, 7);
  }
}