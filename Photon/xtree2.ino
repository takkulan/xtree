/* 
   xtree2 - new year 2016 edition
   	  - fireworks mode
	  - color palette setting with remote callback
	    	  	  	      (31.12.2015 takku)
					*/


// This #include statement was automatically added by the Particle IDE.
#include "FastLED/FastLED.h"
FASTLED_USING_NAMESPACE;
#define DATA_PIN A5   // FOR BREADBORD STRIP RUNNING WITH APA102C
#define CLOCK_PIN A3  // FOR BREADBORD STRIP RUNNING WITH APA102C
#define NUM_LEDS 240  // Length of led strip 144 for one meter 240 for longer, also different RGB order.
#define MAX_PGM 5
#define MAX_EXPLOSIONS 10

#define FLIGHT_DURATION 2000 // durations for explosions: flight time
#define BLAST_DURATION 200  // explosion time
#define SPARCLE_DURATION 3000 // sparcle time



// -------------------Globals -------------------------

CRGB leds[NUM_LEDS];    // this is the display buffer

CRGBPalette16 currentPalette;  //  currently running color palette
TBlendType    currentBlending; //  currently running blending mode

int currentProgram = 0;         // currently running program (=palette) number


long loopTimer =0;

typedef struct {
    int pos;
    int size;
    long startTime;
    CRGB color;
} explosion_s;

explosion_s explosion[MAX_EXPLOSIONS];


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
            FastLED.show();
            delay(100);
            for (int i = 0; i < MAX_EXPLOSIONS;i++){
                explosion[i].startTime = 0;
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
  loopTimer = millis();
}




int shellIndex=0;
// display an explosion at given coordinate
void launch( int pos, int size, int color){
    explosion[shellIndex].pos = pos;
    explosion[shellIndex].size = size;
    explosion[shellIndex].startTime= millis();
    explosion[shellIndex].color = ColorFromPalette( currentPalette, color, 0xff, currentBlending);
    shellIndex++;
    if (shellIndex == MAX_EXPLOSIONS){
        shellIndex = 0;
    }
}



// phases
void flight(int index){
    int flightAge = millis()-explosion[index].startTime;
    int flightPos = (float)flightAge/(float)FLIGHT_DURATION * explosion[index].pos;
    leds[flightPos] += CRGB(0x02,0,0);
}

// phases
void blast(int index){
    int blastAge = millis()-explosion[index].startTime-FLIGHT_DURATION;
    int blastSize = (float)blastAge/(float)BLAST_DURATION * (float) explosion[index].size;
    for (int i=0;i<blastSize*2;i++){
        int blastPoint = explosion[index].pos-blastSize+i;
        if ((blastPoint >= 0) && (blastPoint < NUM_LEDS)){
            leds[blastPoint] += explosion[index].color;
        }
    }
}

// phases
void sparcle(int index){
    int sparcleAge = millis()-explosion[index].startTime-FLIGHT_DURATION-BLAST_DURATION;
    int sparcleTTL = SPARCLE_DURATION - sparcleAge;
    int sparcleProb = (float) sparcleTTL / (float) SPARCLE_DURATION * 100;
    
    if (random8(0, 101-sparcleProb) == 0){
        int sparcleSize = (float)sparcleAge/(float)SPARCLE_DURATION * (float)explosion[index].size + (float)explosion[index].size;
        int sparclePix = random8(sparcleSize);
        int sparclePoint = explosion[index].pos-sparcleSize/2-sparclePix-sparcleAge/50;
        if ((sparclePoint >= 0) && (sparclePoint < NUM_LEDS)){
            leds[sparclePoint] += explosion[index].color;
        }
        sparclePoint = explosion[index].pos+sparcleSize/2+sparclePix-sparcleAge/50;
        if ((sparclePoint >= 0) && (sparclePoint < NUM_LEDS)){
            leds[sparclePoint] += explosion[index].color;
        }
    }
}


// process the explosions
// over time a single explosion goes through states
// flight -> blast -> sparcle
void runner(){
    for (int i=0;i<MAX_EXPLOSIONS;i++){
        int shellEvent = millis() - explosion[i].startTime;
        if (shellEvent < FLIGHT_DURATION){
            flight(i);
        } else if (shellEvent < FLIGHT_DURATION+BLAST_DURATION){
            blast(i);
        } else if (shellEvent < FLIGHT_DURATION+BLAST_DURATION+SPARCLE_DURATION){
            sparcle(i);
        }
    }
}




// -------------------Loop -------------------------
int i=0;  //simple loop counter
void loop(){
  if(currentProgram != 0){ //only run if current program is not off
        if(i++ % 200 == 0){  // just light led on every nth cycle
            launch(random8(50,NUM_LEDS),random8(5,30),random8());
        }
  }
  runner(); // handle all launched fireworks
  if (millis() > loopTimer+2){  // we  dim framebuffer when 2ms has passed
      for (int j=0;j<NUM_LEDS;j++){
        leds[j].nscale8(250);  // dim down whole strip for nice fade effect
        loopTimer=millis();
    }
  }
  FastLED.show();
//  delay(50);  // sets the loop duration to roughly 50ms -> 20 frame updates per second
 
}



