/*  globals.h
      Define all user set values, global constants, global vars, included libraries and global objects

      Imogen Heard
      25/01/2025
*/

/*  Changelog

V0.0.0
Sketch uses 48544 bytes (18%) of program storage space. Maximum is 262144 bytes.
Global variables use 6100 bytes (18%) of dynamic memory, leaving 26668 bytes for local variables. Maximum is 32768 bytes.

*/

#pragma once

// Add included external libraries here (at the top of globals.h)
#include <Wire.h>
#include <SPI.h>
#include <autoDelay.h>    // https://github.com/PanGalacticTech/autoDelay_library
#include <ArduinoJson.h>  // installed version 6.21.5 [Arduino Library Manager]
#include <ledObject.h>
#include "secretObject.h"


// Program Attributes
#define EXPERIMENT_NAME "ripple-tank"
#define FIRMWARE_VERSION "V0.0.0"
#define DEVELOPER "Imogen Heard"


// Hardware Definitions
const int dacPin = A0;  // DAC
const int digiPotSelectPin = 10;
const int lampCtrlPin = 3;
const int beaconPin = 5;  // user interface LED


// User Runtime Adjustable Variables -> These are the variables the user will direct interact with as part of the experiment
uint8_t amplitude = 10;
float frequency = 30;
uint16_t brightness = 100;

bool waveOutput = false;
bool lampOutput = false;

// User Runtime Defaults -> Default values for the runtime variables (if required) for the program to default to after some set time
// -> This can be useful so the equipment can be assumed to be in some defined state if not running for a period of time. Will also be used as the startup variables
uint8_t amplitudeDefault = 30;
float frequencyDefault = 20;
uint16_t brightnessDefault = 100;


// Wavetable Stuff
int tableSize = 240;  // Number of samples in the sine wave table
// constants for different sine tables
const int low_table_size = 240;  // up to 30 Hz  (aprox-> defined below)
const int mid_table_size = 120;  // 30 - 60 Hz
const int high_table_size = 60;  // 60 to 120 Hz
const int VH_table_size = 30;    // 60 to 120 Hz

const int low_table_Hz = 20;   // frequences under this limit will use the low table
const int mid_table_Hz = 40;   // frequences between low and mid will use mid table
const int high_table_Hz = 80;  // frequencies between mid and high limit will use the high table
const int VH_table_Hz = 80;    // frequencies above high will use the Very High table


typedef enum {
  LOW_HZ_TABLE,
  MID_HZ_TABLE,
  HIGH_HZ_TABLE,
  VH_HZ_TABLE
} activeTable;

activeTable currentTable;

float waveBaseDelay_uS = 0;    // delay for 1Hz
int16_t waveDelayTime_uS = 0;  // calculated delay for {frequency}
uint16_t table_index = 0;      //  sample index for the wavetable



// pulse timing variables
uint32_t pulseTime_mS = 0;
uint32_t pulseStartTime_mS;
bool pulseActive = false;


// User Options & program config
#define INIT_SAMPLE_RATE_Hz 2
#define INIT_PRINT_RATE_Hz 2
const int WAVE_TIMEOUT_S = 300;               // Times out running mode/movement after time delay from recieving last command
const int LAMP_TIMEOUT_S = 300;               // Times out running mode/movement after time delay from recieving last command
const int SETTINGS_DEFAULT_TIMEOUT_S = 3600;  // Times out any changes to settings and goes to defaults

#define JSON_TX_BUFFER_SIZE 100
#define STREAMING_DEFAULT_ACTIVE false
#define PRINT_JSON true
#define PRETTY_PRINT_JSON false

#define MAX_BRIGHTNESS 1200
#define MAX_HZ 300

// Error System
#define WARNING_ACTIVE_PERIOD_mS 60000  // 1 min


#define JSON_COMMAND_SIZE 64  // what command find better description


// Debugging Options
#define DEBUG_STATES false        // not JSON safe
#define DEBUG_STATE_MACHINE true  // JSON safe
#define COMMAND_HINTS false

// Physics Constants
#define G_CONST 9.80665


// Utility
#define EEPROM_ENCODE_OFFSET_ADDRESS 0xFF  // The first 15 values in EEprom are used by uStepper lib. 0xFF is arbitraty address away from first 15 addresses




// Add included internal libraries here
#include "jsonMessenger.h"

// Create objects
jsonMessenger jsonRX;  // create a jsonMessenger object to handle commands received over Serial connection
ledObject beacon(beaconPin);
//secretObject memory;
autoDelay waveTableDelay;



//secretObject::calStruc cal;







// Global Variables
// Global Timers
uint32_t last_command_rx_mS = 0;



//EEprom Variables
const int WRITTEN_SIGNATURE = 0x98C7AB1E;  // Arbitary signature to check for existing encoder offset value in persistant memory (practable)


// Sampling Vars
bool streaming_active = STREAMING_DEFAULT_ACTIVE;
bool snapshop_active = false;
uint16_t snapshot_timer_mS = 25000;
uint32_t snapshot_starttime_mS;

#define SAMPLE_DELAY_OFFSET 0  // offset in mS

autoDelay sampleDelay;
uint16_t sampleRate_Hz = INIT_SAMPLE_RATE_Hz;
//uint32_t sampleDelay_mS = uint32_t(1000 / sampleRate_Hz) - 4;  // added -5 to make the delay just a little shorter, and ensure we get all samples in before they are sent.
uint32_t sampleDelay_mS = uint32_t(1E3 / sampleRate_Hz) - SAMPLE_DELAY_OFFSET;  //
// The function will stop sampling once buffer it full, so this should make more consistant outputs
// #TODO MAKE SURE STATE FUNCTION IS UPDATED TO MATCH

autoDelay printDelay;
uint16_t print_rate_Hz = INIT_PRINT_RATE_Hz;
uint32_t print_delay_mS = uint32_t(1000 / print_rate_Hz);

uint8_t num_samples_req = uint8_t(sampleRate_Hz / print_rate_Hz);  // Number of samples required to collect between each print cycle

// Maximum sample rate of 200Hz, minimum print rate of 1Hz, so max number of data to be collected is 200 samples x 8 datapoints, 4 bytes per float = 5600 bytes. Very doable data levels on STM32, HOWEVER;
// remember all this data will need to be copied to JSON so lets double that 11200! -> still very doable on STM32!

// but in the future, this must be restricted
// on smaller microcontrollers. Suggest limit of 4, so print rate cant even be less than 4 times sample rate. This can be defined by DATA_ARRAY_SIZE, and will (later) be used to resitrict sample rates and print rates to
// stay within this value, each time one is updated it must be checked for conformity to this against the other.

#define DATA_ARRAY_SIZE 200

uint32_t timestamp_array[DATA_ARRAY_SIZE];
float encode_array[DATA_ARRAY_SIZE];  // named as such and left here incase needed later


int16_t samples_written = 0;


// Utility functions
void switchVariables(float &varA, float &varB) {
  float b;
  b = varA;
  varA = varB;
  varB = b;
}



// Add included internal header files here (at the bottom of globals.h)

#include "waveMachine.h"
#include "demo.h"
#include "stateConfig.h"
#include "jsonReporter.h"
