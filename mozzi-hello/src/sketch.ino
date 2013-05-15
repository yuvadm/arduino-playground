#include <MozziGuts.h> // at the top of your sketch 
#define CONTROL_RATE 64 // or some other power of 2 

void setup() {
    startMozzi(CONTROL_RATE);
    randomSeed(analogRead(0));
} 

void loop() { 
    audioHook(); // fills the audio buffer 
} 

int updateAudio() {
    // your audio code which returns an int 
    // between -244 and 243 in STANDARD mode
    // or between -8192 and 8191 in HIFI mode
    return randNumber = random(-244, 243);
} 

void updateControl() {
    // your control code 
} 

