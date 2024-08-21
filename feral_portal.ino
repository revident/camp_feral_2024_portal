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

    switch (tagNum) {
      case 408416246182 :
        setScene(0);
        break;
      case 601411383637 :
        setScene(1);
        break;
      case 1337 :
        setScene(2);
        break;
      case 60141224218177 :
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
      sceneSolidRed();
      break;
    case 4 : // Test Scene
      sceneEndCheck();
      break;
    default : // Black Out
      sceneBlackOut();
  }

  // Global Throttle, because the 8 light fixures are unstable at full blast.
  delay(10);
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