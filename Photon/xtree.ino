/* xtree - xmas tree light 
	- slow blinks for random dots
	- remote control for color palette
			(24.12.2015 takku)
				*/


// This #include statement was automatically added by the Particle IDE.
#include "FastLED/FastLED.h"

FASTLED_USING_NAMESPACE;
#define DATA_PIN A5   // FOR BREADBORD STRIP RUNNING WITH APA102C
#define CLOCK_PIN A3  // FOR BREADBORD STRIP RUNNING WITH APA102C
#define NUM_LEDS 240  // Length of led strip 144 for one meter 240 for longer, also different RGB order.
#define MAX_PGM 5

// -------------------Globals -------------------------

CRGB leds[NUM_LEDS];    // this is the display buffer

CRGBPalette16 currentPalette;  //  currently running color palette
TBlendType    currentBlending; //  currently running blending mode

int currentProgram = 0;         // currently running program (=palette) number



// -------------------Callbacks -------------------------
// Callback: Setting running program number 0..5
// input one integer value ranging 0..5
int setProgram(String command){
    int retval =0;
    int curPgm = command.toInt();
    
    if ((curPgm >=0) && (curPgm <= MAX_PGM)){ // sanity check input
    
        if(curPgm != currentProgram){ // show notification on prog change
            for (int j=0;j<NUM_LEDS;j++){
                leds[j] = CHSV(random8(),255,128);
            }   
        }
    
        currentProgram = curPgm;
        switch (currentProgram){
            case 0:
            //off - this is handled in main loop
            break;
            case 1:
            currentPalette = OceanColors_p;
            break;
            case 2:
            currentPalette = CloudColors_p;
            break;
            case 3:
            currentPalette = LavaColors_p;
            break;
            case 4:
            currentPalette = ForestColors_p;
            break;
            case 5:
            currentPalette = PartyColors_p;
            break;
        }
        retval = currentProgram;
    }else{
        retval = -1;
    }
    return retval;
}

// -------------------Setup -------------------------
void setup() {
  Particle.function("setProgram", setProgram);
  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, GBR>(leds, NUM_LEDS);
  fill_solid( &(leds[0]), NUM_LEDS, CRGB::Black);
  FastLED.show();  // just a sanitary clearing of the strip, likely not needed
  
  currentProgram = 1;
  currentPalette = PartyColors_p;
  currentBlending = LINEARBLEND;
}


// -------------------Loop -------------------------
int i=0;  //simple loop counter

void loop(){
  if(currentProgram != 0){ //only run if current program is not off
        if((i++ % 4)==0){  // just light led on every nth cycle
            // show a colorful mode-change-feedback
         leds[random8(NUM_LEDS)]=ColorFromPalette( currentPalette, random8(), 0xff, currentBlending);
        }
  }
  for (int j=0;j<NUM_LEDS;j++){
    leds[j].nscale8(254);  // dim down whole strip for nice fade effect
  }
  FastLED.show();
  delay(50);  // sets the loop duration to roughly 50ms -> 20 frame updates per second
}
