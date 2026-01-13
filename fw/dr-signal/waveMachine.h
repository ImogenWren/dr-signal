/* Wave Machine

  - All functions relevent to ripple tank /wave machine function
  
   
// Relevent Global Vars (find in globals.h)
typedef uint8_t enum {
  LOW_HZ_TABLE,
  MID_HZ_TABLE,
  HIGH_HZ_TABLE
} activeTable;

// Wavetable Stuff
const int tableSize = 240;  // Number of samples in the sine wave table
// constants for different sine tables
const int low_table_size = 240;  // up to 30 Hz  (aprox-> defined below)
const int mid_table_size = 120;  // 30 - 60 Hz
const int high_table_size = 60;  // 60 to 120 Hz


const int low_table_Hz = 20;    // frequences under this limit will use the low table   
const int mid_table_Hz = 40;     // frequences between low and mid will use mid table
const int high_table_Hz = 40;    // frequencies above this limit will use the high table


typedef uint8_t enum {
  LOW_HZ_TABLE,
  MID_HZ_TABLE,
  HIGH_HZ_TABLE
} activeTable;

float waveBaseDelay_uS = 0;    // delay for 1Hz
int16_t waveDelayTime_uS = 0;  // calculated delay for {frequency}
uint16_t table_index = 0;       //  sample index for the wavetable

Use the python script `sine-wavetable-generator.py` within the fw folder of this repo to generate sine tables of the correct size for this firmware


*/

#pragma once

#include "waveTables.h"

// volume for output using digital pot
void setVolume(const uint8_t &volume) {
  digitalWrite(digiPotSelectPin, LOW);
  unsigned char MSb = 0x0;
  unsigned char LSb = volume & 0xFF;  // no value greater than 255, else send 0x00
  uint16_t transmission = MSb << 8 | LSb;
  SPI.transfer16(transmission);
  digitalWrite(digiPotSelectPin, HIGH);
}


uint32_t set_frequency(float frequency = 1) {
  waveDelayTime_uS = uint32_t((waveBaseDelay_uS / frequency) + 0.5);  // waveBaseDelay_uS = 3906 previously
  return waveDelayTime_uS;
}


void set_brightness(uint16_t brightness) {
  analogWrite(lampCtrlPin, brightness);
}


// Used at startup to calculate the delay time between samples at 1Hz
void calc_wave_baseDelay(uint16_t num_entries) {
  waveBaseDelay_uS = 1000000.0 / float(num_entries);
  // Serial.print("Base Delay uS: ");
  // Serial.println(waveBaseDelay_uS);
}

void select_wavetable(float frequency = 1) {
  if (frequency <= low_table_Hz) {
    currentTable = LOW_HZ_TABLE;
    tableSize = low_table_size;
  } else if (frequency > low_table_Hz && frequency <= mid_table_Hz) {
    currentTable = MID_HZ_TABLE;
    tableSize = mid_table_size;
  } else if (frequency > mid_table_Hz) {
    currentTable = HIGH_HZ_TABLE;
    tableSize = high_table_size;
  } else {  // default // error? // default to low table
    currentTable = LOW_HZ_TABLE;
    tableSize = low_table_size;
    Serial.println("ERROR");
  }
}




void wave_tank_setup() {
  // Set analog write to 10 bits to activate DAC output on pin A0
  analogWriteResolution(10);
  // Set up LED lamp and set brightness to 0
  pinMode(lampCtrlPin, OUTPUT);
  set_brightness(0);
  // Set Up SPI for digital pot volume control and set volume to 0
  pinMode(digiPotSelectPin, OUTPUT);
  SPI.begin();
  setVolume(0);
}






void run_wavetable() {
  if (waveOutput) {
    uint16_t tableVal = 0;
    if (waveTableDelay.microsDelay(waveDelayTime_uS)) {
      if (table_index >= tableSize) table_index = 0;  // reset this first as it will avoid indexes going OOB
      if (currentTable == VH_HZ_TABLE) {
        tableVal = pgm_read_word(&sineTable_Vhigh[table_index]);
      } else if (currentTable == HIGH_HZ_TABLE) {
        tableVal = pgm_read_word(&sineTable_high[table_index]);
      } else if (currentTable == MID_HZ_TABLE) {
        tableVal = pgm_read_word(&sineTable_mid[table_index]);
      } else if (currentTable == LOW_HZ_TABLE) {
        tableVal = pgm_read_word(&sineTable_low[table_index]);
      } else {
        tableVal = 255;  //stable mid scale output will highlight this error
      }
      analogWrite(dacPin, tableVal);
      // Serial.println(tableVal);
      table_index++;
    }
  }
}
