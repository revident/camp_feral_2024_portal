/* RFID Setup */
#include <Wire.h>
#include <SparkFun_Qwiic_Rfid.h>

#define RFID_ADDR 0x13 // Default I2C address 

Qwiic_Rfid myRfid(RFID_ADDR);

// Interrupt Pin on pin 6. 
// const int intPin = 6; // not used, polling instead
String tag;
const long rfidPollIntvl = 1000; //milliseconds
long nextRfidPoll = 0;

/* DMX Setup */
#include <DmxOutput.h>
#include <DmxOutput.pio.h>

// Declare an instance of the DMX Output
// DmxOutput dmx;

#define UNIVERSE_LENGTH 68 // 4 + 32 + 32 RGBW channels
uint8_t universe[UNIVERSE_LENGTH + 1];

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

void sceneBlackOut() {
  for (int i = 1; i < UNIVERSE_LENGTH + 1; i++) {
    universe[i] = 0;
  }
}

void sceneChannelStrobe() {
 for (int i = 1; i < UNIVERSE_LENGTH + 1; i++) {
    universe[i] = 127;
  }
}

void sceneSolidColorHalfB(uint8_t _rgbwChannel) {
  /* Take a number representing Red[1], Green[2], Blue[3] or White[4] Channels
  Set all channels of the color to half brightness */
  for (int i = _rgbwChannel; i < UNIVERSE_LENGTH + 1; i+=4) {
    universe[i] = 127;
  }
}

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

    long long tagNum = std::strtoll(tag.c_str(), nullptr, 10);

    switch (tagNum) {
      case 6014114812499 : // Black
        setScene(0);
        break;
      case 60141224218177 : // Red
        setScene(1);
        break;
      case 408416246182 : // Green
        setScene(2);
        break;
      case 601411383637 : // Blue
        setScene(3);
        break;
      case 601411495739 : // White
        setScene(4);
        break;
    }
  }


// https://arduino-pico.readthedocs.io/en/latest/multicore.html
// push DMX scene changes to fifo

  digitalWrite(ledPin, LOW); // stop heartbeat LED
}

void setScene(uint32_t _sceneNum) {
  rp2040.fifo.push(_sceneNum);
}

void setup1() {
  //dmx.begin(dmxPin);

  FastLED.addLeds<DMXPICO, dmxPin, RGB>(leds, NUM_LEDS);
}

void loop1() {

  // read DMX scene changes from FIFO
  if (rp2040.fifo.available()) {
    rp2040.fifo.pop_nb(&activeScene);
    
    switch (activeScene) {
      case 1 : // Red
        sceneBlackOut();
        sceneSolidColorHalfB(1);
        break;
      case 2 : // Green
        sceneBlackOut();
        sceneSolidColorHalfB(2);
        break;
      case 3 : // Blue
        sceneBlackOut();
        sceneSolidColorHalfB(3);
        break;
      case 4 : // WhitesceneBlackOut();
        sceneBlackOut();
        sceneSolidColorHalfB(4);
        break;
      default : // BlackOut
        sceneBlackOut();
    }
    Serial.println("Switched to DMX scene " + String(activeScene)); 
  }
  
  leds[0] = CRGB::White; FastLED.show(); delay(300);
  leds[0] = CRGB::Black; FastLED.show(); delay(300);
  
  /*dmx.write(universe, UNIVERSE_LENGTH + 1);
  while(dmx.busy()) {
    // Patiently wait, or do other computing stuff
  }*/

}