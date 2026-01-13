/*  stateConfig.h

This header defines the state machine framework. It is intended to be used with the jsonMessenger.h Library to handle receiving and parsing JSON formatted commands entered by a user, 
however this state machine also operates independently and can be used without jsonMessenger.h

This State Machine framework can be adapted to many different purposes, follow the numbered comments for an explanation

Imogen Heard
11/02/2025


*/

#pragma once

#include "globals.h"


// 1. Define all the valid states for the state machine with an enum.
//    - This should include states triggered by user input, but may also contain states that are only accessable programatically
typedef enum {
  STATE_INIT,
  STATE_WAIT,
  STATE_STOP,
  STATE_START,
  STATE_SET_HZ,
  STATE_LAMP,
  STATE_AMP,
  STATE_PULSE,
  STATE_SAMPLERATE,
  STATE_PRINTRATE,
  STATE_STARTSTREAM,
  STATE_STOPSTREAM,
  STATE_SNAPSHOT,
  STATE_SNAPTIME,
  STATE_SETSECRET,
  STATE_SETCAL,
  STATE_GETCAL,
  STATE_DEMO,
  STATE_HELP,
  NUM_STATES  // Sentinal value lets us get the total number of states without manually counting. Do not forget this value, it is important for correct function
} StateType;


// 2. Then we define two variables to hold the current state enum, and the previous state enum. Holding both these values allows us to compare them as we enter a state,
//    to see if it is the first time we have entered the state.

StateType smState = STATE_INIT;
StateType lastState = NUM_STATES;




// 3. Define a State Names Array, this will allow us to print the enum above in human readable format
char stateNames[][20] = {
  "STATE_INIT",
  "STATE_WAIT",
  "STATE_STOP",
  "STATE_START",
  "STATE_SET_HZ",
  "STATE_LAMP",
  "STATE_AMP",
  "STATE_PULSE",
  "STATE_SAMPLERATE",
  "STATE_PRINTRATE",
  "STATE_STARTSTREAM",
  "STATE_STOPSTREAM",
  "STATE_SNAPSHOT",
  "STATE_SNAPTIME",
  "STATE_SETSECRET",
  "STATE_SETCAL",
  "STATE_GETCAL",
  "STATE_DEMO",
  "STATE_HELP"
};


// 4. Define the state machine function prototypes.
//    Any function that will be passed data from a user input will be passed the same jsonStateData structure as an argument.
// Experiment Specific states
void sm_state_init(void);
void sm_state_wait(void);
void sm_state_stop(void);
void sm_state_start(void);
void sm_state_set_hz(jsonStateData stateData);
void sm_state_lamp(jsonStateData stateData);
void sm_state_amp(jsonStateData stateData);
void sm_state_pulse(jsonStateData stateData);
// Generic, Common & Utility states
void sm_state_samplerate(jsonStateData stateData);
void sm_state_printrate(jsonStateData stateData);
void sm_state_start_stream(jsonStateData stateData);
void sm_state_stop_stream(void);
void sm_state_snapshot(void);
void sm_state_snaptime(jsonStateData stateData);
void sm_state_setsecret(jsonStateData stateData);
void sm_state_setcal(jsonStateData stateData);
void sm_state_getcal(jsonStateData stateData);
void sm_state_demo(jsonStateData stateData);
void sm_state_help(void);





// 5. Define all the state machine functions
//    - A typical state function template is shown below

/*
void sm_state_template(jsonStateData stateData) {
  if (lastState != smState) {  // Check to see if first time state has been called in sequence
    // 5a. Do anything that needs to happen the First time this state is called
    lastState = smState;  // Set the last state to the current state
  }
  // 5b. Do everything that repeats as long as this state is active

  // 5c. Extract data from jsonStateData (if required)
          - remember to validate data if coming from user input!

  if (stateData.numeric < 1 || stateData.numeric > 2000){
    // Error message to user if data is out of accepted bounds
  } else {
    // accept the data and process
  }

  // 5d. Add any logic to navigate to other states
  smState = STATE_WAIT;
}
*/



// Init state, only called at startup
void sm_state_init() {
  if (lastState != smState) {
#if DEBUG_STATES == true
    Serial.println(F("state: INIT"));
#endif
  }
  amplitude = amplitudeDefault;
  setVolume(amplitude);
  frequency = frequencyDefault;
  select_wavetable(frequency);
  calc_wave_baseDelay(tableSize);
  set_frequency(frequency);
  brightness = brightnessDefault;
  set_brightness(brightness);

  smState = STATE_WAIT;
}


// Not a state, but function called by "help" state to print commands list to users
void print_cmds() {
  Serial.println(F("   {\"set\":\"start\"}                     -> Start wave output to speaker"));
  Serial.println(F("   {\"set\":\"stop\"}                      -> Stop wave output to speaker "));
  Serial.println(F("   {\"set\":\"hz\",\"to\": 0 to 600 }      -> Set Wave Frequency   "));
  Serial.println(F("   {\"set\":\"lamp\",\"to\": 0 or 1024 }   -> Set LED Brightness "));
  Serial.println(F("   {\"set\":\"amp\",\"to\": 0 to 255 }     -> Set wave amplitude "));
  Serial.println(F("   {\"set\":\"pulse\",\"to\": 0 to {uint32}}-> Pulse output for {_} mS "));
  Serial.println(F("   {\"set\":\"stream\"}                     -> Start Data Streaming   "));
  Serial.println(F("   {\"set\":\"endst\"}                      -> End Data Streaming "));
  Serial.println(F("   {\"set\":\"snap\"}                       -> Take Data Snapshot   "));
  Serial.println(F("   {\"set\":\"time\",\"to\": 1 - 250000 }   -> Set Time for Data Snapshot (mS)  "));
  Serial.println(F("   {\"set\":\"setcal\", \"to\":\"0 - 32768\", \"auth\":\"XXXXXXXX\"} -> Set calibration offset"));   // Note, when changing print & sample rates, the size of the JSON doc may not be able to handle additional data. Max number of samples is governed by JSON doc size
  Serial.println(F("   {\"set\":\"help\"}                       -> Print Commands to Serial Monitor  "));
}


void sm_state_setsecret(jsonStateData stateData) {
#if DEBUG_STATES == true
  Serial.println(F("state: SECRET"));
#endif
  lastState = smState;
  // Serial.print("secret: ");
  // Serial.println(stateData.msg);
  if (stateData.msg[0] == '\0') {  //empty string
    smState = STATE_WAIT;
    return;
  }
  // memory.cal_set_secret(stateData.msg);

  smState = STATE_WAIT;
}


void sm_state_setcal(jsonStateData stateData) {
#if DEBUG_STATES == true
  Serial.println(F("state: SETCAL"));
#endif
  lastState = smState;
  // code here
  //  Serial.print("cal data: ");
  // Serial.print(stateData.numeric);
  // Serial.print(" auth: ");
  // Serial.println(stateData.msg);

  // memory.cal_set_values(stateData.numeric, stateData.msg);
  smState = STATE_WAIT;
}



void sm_state_getcal(jsonStateData stateData) {
#if DEBUG_STATES == true
  Serial.println(F("state: GETCAL"));
#endif
  lastState = smState;
  //  cal = memory.get_cal();
  // if (cal.calValid) {
  // persistant_encoder_offset = cal.calData;
  //  } else {
  // persistant_encoder_offset = 0;
  // }

  smState = STATE_WAIT;
}




// State Wait is the default state for this program
void sm_state_wait() {
  if (lastState != smState) {
    // If first iteration print state machine status
#if DEBUG_STATES == true
    Serial.println(F("state: WAIT"));
#endif
#if COMMAND_HINTS == true  // print suggested commands
    Serial.println(F("\nEnter cmd in format:"));
    print_cmds();
#endif
    lastState = smState;
  }
  // smState = STATE_STOP;
}




// Stop the wave machine
void sm_state_stop(void) {
  if (lastState != smState) {
#if DEBUG_STATES == true
    Serial.println(F("state: STOP"));
#endif
    lastState = smState;
  }
  waveOutput = false;
  smState = STATE_WAIT;
}


// Start the wave machine
void sm_state_start(void) {
  if (lastState != smState) {
#if DEBUG_STATES == true
    Serial.println(F("state: START"));
#endif
    lastState = smState;
    waveOutput = true;
  }

  if (STREAMING_DEFAULT_ACTIVE) {
    smState = STATE_STARTSTREAM;
  } else {
    smState = STATE_WAIT;
  }
}





// Set the Motor Speed in Hz
void sm_state_set_hz(jsonStateData stateData) {
  if (lastState != smState) {
#if DEBUG_STATES == true
    Serial.println(F("state: SET_HZ"));
#endif
    lastState = smState;
  }
  //  if (stateData.floatData < (-1 * MAX_HZ) || stateData.floatData > MAX_HZ) {  // if value is out of range, reject
  //   Serial.println(F("{\"WARNING\":\"Requested Hz value out of bounds\"}"));
  //  } else {
  Serial.print("{\"hz-set-to\":\"");
  Serial.print(stateData.floatData);
  Serial.println("\"}");
  frequency = stateData.floatData;
  select_wavetable(frequency);     // select the correct wavetable for the frequency
  calc_wave_baseDelay(tableSize);  // calculate the default delay for 1Hz
  set_frequency(frequency);        // divide the default delay by the frequency to get the delay for this frequency
  //  }
  smState = STATE_WAIT;
}






// Set the Lamp Output Brightness
void sm_state_lamp(jsonStateData stateData) {
  if (lastState != smState) {
#if DEBUG_STATES == true
    Serial.println(F("state: LAMP"));
#endif
    lastState = smState;
  }
  brightness = stateData.numeric;
  set_brightness(brightness);
  smState = STATE_WAIT;
}


// Run motor home position calibration and homing scripts
void sm_state_amp(jsonStateData stateData) {
  if (lastState != smState) {
#if DEBUG_STATES == true
    Serial.println(F("state: AMP"));
#endif
    lastState = smState;
  }
  amplitude = stateData.numeric;
  setVolume(amplitude);
  smState = STATE_WAIT;
}



// Run motor home position calibration and homing scripts
void sm_state_pulse(jsonStateData stateData) {
  if (lastState != smState) {
#if DEBUG_STATES == true
    Serial.println(F("state: PULSE"));
#endif
    lastState = smState;
    pulseActive = true;
    pulseTime_mS = stateData.numeric;
    waveOutput = true;
    pulseStartTime_mS = millis();
  }

  smState = STATE_WAIT;
}




// Change the samplerate of streamed or snapshotted data (Init at 10 Hz)
void sm_state_samplerate(jsonStateData stateData) {
  if (lastState != smState) {
#if DEBUG_STATES == true
    Serial.println(F("state: SAMPLERATE"));
#endif
    lastState = smState;
  }
  if (stateData.numeric < 1 || stateData.numeric > 200) {
    Serial.println(F("{\"WARNING\":\"Out of Bounds Sample Rate Commanded\"}"));
  } else if (stateData.numeric < print_rate_Hz) {
    Serial.println(F("{\"WARNING\":\"Cannot set sample rate less than print rate\"}"));
  } else {
    sampleRate_Hz = stateData.numeric;
    // sampleDelay_mS = 1000 / sampleRate_Hz;
    sampleDelay_mS = 1E3 / sampleRate_Hz - SAMPLE_DELAY_OFFSET;
    num_samples_req = uint8_t(sampleRate_Hz / print_rate_Hz);  // Number of samples required to collect between each print cycle
  }
  smState = STATE_WAIT;
}



// Change the samplerate of streamed or snapshotted data (Init at 10 Hz)
void sm_state_printrate(jsonStateData stateData) {
  if (lastState != smState) {
#if DEBUG_STATES == true
    Serial.println(F("state: PRINT RATE"));
#endif
    lastState = smState;
  }
  if (stateData.numeric < 1 || stateData.numeric > 50) {
    Serial.println(F("{\"WARNING\":\"Out of Bounds Print Rate Commanded\"}"));
  } else if (stateData.numeric > sampleRate_Hz) {
    Serial.println(F("{\"WARNING\":\"Cannot set print rate greater than sample rate\"}"));
  } else {
    print_rate_Hz = stateData.numeric;
    print_delay_mS = 1000 / print_rate_Hz;
    num_samples_req = uint8_t(sampleRate_Hz / print_rate_Hz);  // Number of samples required to collect between each print cycle
  }
  smState = STATE_WAIT;
}





// Start streaming data
void sm_state_start_stream() {
  if (lastState != smState) {
#if DEBUG_STATES == true
    Serial.println(F("state: STARTSTREAM"));
#endif
    lastState = smState;
  }
  streaming_active = true;
  //  snapshot_timer_mS = jsonStateData.numeric;
  smState = STATE_WAIT;
}



// Stop streaming data
void sm_state_stop_stream(void) {
  if (lastState != smState) {
#if DEBUG_STATES == true
    Serial.println(F("state: STOPSTREAM"));
#endif
    lastState = smState;
  }
  streaming_active = false;
  // snapshot_timer_mS = 0;
  smState = STATE_WAIT;
}


// Trigger a snapshot of data to be taken over X amount of time
void sm_state_snapshot() {
  if (lastState != smState) {
#if DEBUG_STATES == true
    Serial.println(F("state: SNAPSHOT"));
#endif
    lastState = smState;
  }
  snapshop_active = true;
  snapshot_starttime_mS = millis();
  smState = STATE_WAIT;
}


// Set the period over which snapshot data is recorded (init to 25 seconds)
void sm_state_snaptime(jsonStateData stateData) {
  if (lastState != smState) {
#if DEBUG_STATES == true
    Serial.println(F("state: SNAPTIME"));
#endif
    lastState = smState;
  }
  if (stateData.numeric < 1 || stateData.numeric > 250000) {
    //errors.set_error(false, -10, "Out of Bounds Snapshot Time Commanded", errors.WARNING, "state-snaptime");
    Serial.println(F("{\"WARNING\":\"out of bounds snapshot time requested\"}"));
    //errors.print_json_status();
  } else {
    snapshot_timer_mS = stateData.numeric;
  }
  smState = STATE_WAIT;
}








void sm_state_demo(jsonStateData stateData) {
  if (lastState != smState) {
#if DEBUG_STATES == true
    Serial.println(F("state: DEMO"));
#endif
    lastState = smState;
    // beacon.startBlink(600, 100);
    // stepper.setRPM(1);
    // motorState == RUNNING;
    demoState = 1;
  }
  int16_t exit = demoSM();
  if (exit < 0) {
    smState = STATE_WAIT;
  }
}


// Print the commands list to the Serial Output
void sm_state_help(void) {
  if (lastState != smState) {
#if DEBUG_STATES == true
    Serial.println(F("state: help"));
#endif
    lastState = smState;
  }
  print_cmds();
  smState = STATE_WAIT;
}




// 6. Finally define the state machine function
//    - Automatically generate the switch case from the list of ENUM states and list of functions! -> https://github.com/ImogenWren/switch-case-generator
void sm_Run(jsonStateData stateData) {
  if (smState < NUM_STATES) {
#if DEBUG_STATE_MACHINE == true
    if (lastState != smState) {
      Serial.print(F("{\"statemachine\":\""));
      Serial.print(stateNames[smState]);
      Serial.println(F("\"}"));
    }
#endif
    switch (smState) {
      case STATE_INIT:
        sm_state_init();
        break;
      case STATE_WAIT:
        sm_state_wait();
        break;
      case STATE_STOP:
        sm_state_stop();
        break;
      case STATE_START:
        sm_state_start();
        break;
      case STATE_SET_HZ:
        sm_state_set_hz(stateData);
        break;
      case STATE_LAMP:
        sm_state_lamp(stateData);
        break;
      case STATE_AMP:
        sm_state_amp(stateData);
        break;
      case STATE_PULSE:
        sm_state_pulse(stateData);
        break;
      case STATE_SAMPLERATE:
        sm_state_samplerate(stateData);
        break;
      case STATE_PRINTRATE:
        sm_state_printrate(stateData);
        break;
      case STATE_STARTSTREAM:
        sm_state_start_stream();
        break;
      case STATE_STOPSTREAM:
        sm_state_stop_stream();
        break;
      case STATE_SNAPSHOT:
        sm_state_snapshot();
        break;
      case STATE_SNAPTIME:
        sm_state_snaptime(stateData);
        break;
      case STATE_SETSECRET:
        sm_state_setsecret(stateData);
        break;
      case STATE_SETCAL:
        sm_state_setcal(stateData);
        break;
      case STATE_GETCAL:
        sm_state_getcal(stateData);
        break;
      case STATE_DEMO:
        sm_state_demo(stateData);
        break;
      case STATE_HELP:
        sm_state_help();
        break;
      default:
        sm_state_stop();
        Serial.println(F("sm: Exception -> default"));
        break;
    }
  } else {
    sm_state_stop();
    Serial.println(F("{STATE: \"Exception -> NUM_STATES exceeded\"}"));
  }
}
