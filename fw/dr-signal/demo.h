/* demo.h

 State machine to run entirely within some other state, and show off some of the dynamic features of the experiment hardware

 Imogen 15/07/2025

 - Ensure that all functions are non-blocking && -> exit on RX of command <- this is done in main loop

*/
#include "globals.h"

int16_t demoState = 0;


// function to end demo quickly and return all hardware back to useable state (should be called after receiving any message if demo mode is active)
void end_demo() {
  // do anything to end/reset from demo mode
  beacon.stopBlink();
  demoState = -1;
}



int16_t demoSM(){
  return -1;  // signals exit to demo state
}