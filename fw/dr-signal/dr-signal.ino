/*  Ripple Tank Firmware V0.0.0 - Prototype

_Control firmware for Ripple Tank PCB (prototype)_

- Set Output Frequency for Sine wave using multiple frequency dependent wavetables
- Set output volume of signal provided to class D amp IC using digital pot and SDI
- Set brightness of LED using femotobuck style LED driver

- Measure local ambient temperature, pressure & humidity


Written By:
Imogen Heard
2025-09-30



*/




#include "globals.h"







void setup() {
  Serial.begin(115200);
  // while (!Serial) {          // this will hang waiting for connection (good for final deployment)
  //   delay(1);  // give time for Serial object to start
  //  }
  delay(2000);  // this makes sure program starts even if no connection

  Serial.print("\n{\"model\":\"");
  Serial.print(EXPERIMENT_NAME);
  Serial.print("\",\"version\":\"");
  Serial.print(FIRMWARE_VERSION);
  Serial.print("\",\"fw-developed-by\":\"");
  Serial.print(DEVELOPER);
  Serial.println("\"}");

  wave_tank_setup();  // set up IOs and turn everything to 0

  jsonRX.jsonBegin();  // Start the json library to accept commands over serial connection




  // recall calibration (if required)
  // cal = memory.get_cal();
  //  if (cal.calValid) {
  //    persistant_encoder_offset = cal.calData;//
  //  } else {
  //    persistant_encoder_offset = 0;
  //  }


  beacon.begin(true);  // blink beacon to show ready for use
  beacon.setDefault(true);
  beacon.callBlink();
}



void loop() {


  jsonStateData nextState_data = jsonRX.jsonReadSerialLoop();



  if (nextState_data.cmd_received) {  // If command is receive   //delay(10);

    if (smState == STATE_DEMO) {  // if command is received and program is in demo state, immediatly end
      end_demo();
    }

    beacon.callBlink(4);  // blink beacon to show command RXed

    last_command_rx_mS = millis();  // record time cmd last RXed

    const char* cmd = jsonRX.getCMDkey(nextState_data.cmdState);  // I feel like the entire point of using ENUMs is being totally lost by doing this, but it is working

    // This section here is mostly for debugging, it could be excluded and the function would stay the same
    //>>---------------------->>
    Serial.print("{\"rx-cmd\":\"");
    Serial.print(cmd);
    Serial.print("\",\"datatype\":\"");
    Serial.print(jsonRX.getDataType(nextState_data.data_type));
    Serial.print("\",\"data\":\"");
    if (nextState_data.data_type == INTEGER) Serial.print(nextState_data.numeric);  //std::cout << nextState_data.numeric;
    if (nextState_data.data_type == FLOAT) Serial.print(nextState_data.floatData);  //std::cout << nextState_data.floatData;
    if (nextState_data.data_type == CSTRING) Serial.print(nextState_data.msg);      //std::cout << nextState_data.msg;
    if (nextState_data.data_type == EMPTY) Serial.print("n/a");                     //std::cout << "n/a";
    Serial.println("\"}");
    //<<--------------<<

    // This is the bit that parses the command recieved by user, and sets the state machine to go to the correct state
    switch (nextState_data.cmdState) {
      case STOP:
        smState = STATE_STOP;
        break;
      case START:
        smState = STATE_START;
        break;
      case SET_HZ:
        smState = STATE_SET_HZ;
        break;
      case SET_LAMP:
        smState = STATE_LAMP;
        break;
      case SET_AMP:
        smState = STATE_AMP;
        break;
      case PULSE:
        smState = STATE_PULSE;
        break;
      case SAMPLERATE:
        smState = STATE_SAMPLERATE;
        break;
      case PRINTRATE:
        smState = STATE_PRINTRATE;
        break;
      case STARTSTREAM:
        smState = STATE_STARTSTREAM;
        break;
      case STOPSTREAM:
        smState = STATE_STOPSTREAM;
        break;
      case SNAPSHOT:
        smState = STATE_SNAPSHOT;
        break;
      case SNAPTIME:
        smState = STATE_SNAPTIME;
        break;
      case SETSECRET:
        smState = STATE_SETSECRET;
        break;
      case SETCAL:
        smState = STATE_SETCAL;
        break;
      case GETCAL:
        smState = STATE_GETCAL;
        break;
      case DEMO:
        smState = STATE_DEMO;
        break;
      case HELP:
        smState = STATE_HELP;
        break;
      default:
        // std::cout << "{\"WARNING\":\"Unrecognised cmdState\"}" << std::endl;
        Serial.println("{\"WARNING\":\"Unrecognised cmdState (loop)\"}");
        break;
    }
  }



  sm_Run(nextState_data);  // This Runs the state machine in the correct state, and is passed all of the data sent by the last command


  run_wavetable();  // runs the hardware output that requires periodic updates



  // Do sampling Data at the specified rate
  if (sampleDelay.millisDelay(sampleDelay_mS)) {                                   // added OR if samples written has been reset to 0 after printing || samples_written == 0
    if (samples_written < num_samples_req && samples_written < DATA_ARRAY_SIZE) {  // check to make sure collecting the correct number of samples for the samplerate, and smaller than the
      timestamp_array[samples_written] = millis();
      //encode_array[samples_written] = stepper.encoder.getAngle();
      samples_written++;
    }
    // else if (samples_written == DATA_ARRAY_SIZE) {
    // we have written our last sample to the array, if it was a string, would append with a /n
    // but as floats this isnt needed
    // }
  }

  // do streaming data at the specified rate
  if (streaming_active || snapshop_active) {
    // if (printDelay.millisDelay(print_delay_mS)) {
    if (samples_written >= num_samples_req) {  // REMOVED PRINT TIMER
      sampleDelay.resetDelayTime_mS();         // makes sure that the sample loop is synced to the printing loop //moved to try and improve timings (doing this first so next sample is sooner)
      //print the sampled data
      update_json(samples_written);
      samples_written = 0;
    }
    if (snapshop_active) {
      if (millis() - snapshot_starttime_mS >= snapshot_timer_mS) {
        snapshop_active = false;
      }
    }
  }


  // timeout function example
  // if (servo.attached()) {
  //   if (millis() - servo_attach_time_mS >= SERVO_TIMEOUT_mS) {
  //     servo.detach();
  //    }
  //  }


  // Timeouts & stall check for running state

  if (pulseActive) {
    if (millis() - pulseStartTime_mS >= pulseTime_mS) {
      pulseActive = false;
      smState = STATE_STOP;
    }
  }


  if (waveOutput) {
    if (millis() - last_command_rx_mS >= WAVE_TIMEOUT_S * 1000) {  // running mode timeout
      Serial.println(F("{\"WARNING\":\"Wave - Time Out\"}"));
      beacon.callBlink(8, 500, 500);
      smState = STATE_STOP;
    }
  }


  // if (millis() - last_command_rx_mS >= LAMP_TIMEOUT_S * 1000) {  // running mode timeout
  //   Serial.println(F("{\"WARNING\":\"Lamp - Time Out\"}"));
  //   beacon.callBlink(8, 500, 500);
  //   smState = STATE_STOP;
  //  }

  beacon.performBlink();  // loop function for the LED beacon
}
