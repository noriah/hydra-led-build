#include "FastLED.h"

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#endif

#define LED_TYPE    DOTSTAR
#define COLOR_ORDER BGR
#define NUM_LEDS    30
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          50
#define FRAMES_PER_SECOND   120
#define CHANGE_INTERVAL     70

#define NOSERIAL_QUIT_TIME  5000

int   bugLevel  = 0;              //  This is just a way I manage turning debugging over serial on/off

const char header[] = "PStart";

uint8_t gHue = 0;
uint32_t serialCheck = 0;
unsigned long timeCheck = 0;

boolean hasSerial = true;

void setup() {
  //delay(3000);

  randomSeed(analogRead(A0));
  gHue = random(0,255);

  FastLED.addLeds<LED_TYPE, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  FastLED.setBrightness(BRIGHTNESS);

  Serial.begin(115200);          //  Start the Serial communication

  fill_solid( leds, NUM_LEDS, CRGB::Black);
  FastLED.show();

  while (!Serial.available()) {
    serialCheck++;
    if (serialCheck >= 100) {
      hasSerial = false;
      break;
    } else {
      FastLED.delay(10);
    }
  }

  if (hasSerial) {
    fill_solid( leds, NUM_LEDS, CRGB::White);
    FastLED.show();
  }
  bugit("Setup Complete", 10);    //  DEBUG: Confirm setup is complete
}

void loop()
{
  if (hasSerial) {
    bugit("Waiting for Header", 10);     // DEBUG: Header waiting started
    if (!waitForStartHeader())             // Header check function
    {
      bugit("Start Triggered", 10);     //  DEBUG: Header found; getting color data

      for (int pixCount = 0; pixCount < NUM_LEDS; pixCount++)       // Do this for as many Pixels defined in PixelCount
      {
        for (int color = 0; color < 3; color++)     // For the next 3 incoming bytes
        {
          while (Serial.available() < 1) {}
          leds[pixCount][color] = Serial.read();
        }
      }
      FastLED.show();
    }
  } else {
    if (Serial.available()) {
      hasSerial = true;
      return;
    }

    EVERY_N_MILLISECONDS( CHANGE_INTERVAL ) { gHue++; }
    fill_rainbow( leds, NUM_LEDS, gHue, 7);
    FastLED.show();
    FastLED.delay(1000/FRAMES_PER_SECOND);
  }
}

/**
 *  FUNC    bugit           [manages printing of debugging based on debugging level]
 *  @param  String  bugstr  [string to be be printed]
 *  @param  int     blevel  [the level at which this debugging should be ignored]
 *
*/


int bugit(String bugstr,int blevel)
{
  if (blevel < bugLevel) {
    Serial.println(bugstr);
  }
  return 0;
}

int waitForStartHeader()
{

  bugit("Waiting Loop",10);

  char buffer[3];
  int index = 0;
  timeCheck = millis();

  while (true) {

    while(!Serial.available()){
      if (millis() - timeCheck >= NOSERIAL_QUIT_TIME) {
        hasSerial = false;
        return 1;
      }
    }

    int inByte = Serial.read();

    if(inByte == -1) {
      continue;
    }

    buffer[index] = inByte;
    if(buffer[index] != header[index]) {
      index=-1;                     // not the right sequence restart
    }

    buffer[index+1] = 0;              // add null
    index++;

    if(index==6) {
      return 0;
    }
  }
}
