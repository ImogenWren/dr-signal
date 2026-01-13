/*  Precursor to dr-signal before she gets her PhD

- Only current output with basic display
- Almost perfectly calibrated output

Imogen Wren
13/01/26


*/


#include "globals.h"





void read_pot() {
  int pot_read = 0;
  for (int i = 0; i < samples; i++) {
    pot_read += analogRead(pot_pin);
  }
  pot_read = pot_read / samples;
  if (abs(pot_read - last_pot_read) >= 6) {
    value_changed = true;
    last_pot_read = pot_read;
    if (pot_read <= 10) {
      current_output = 0;
    } else {
      current_output = (pot_read * 0.015779) + 4;    
    }
    Serial.print("pot: ");
    Serial.print(pot_read);
    Serial.print(", current: ");
    Serial.print(current_output);
    // Serial.println();
  }
}

void output_current(float current_mA) {
  int output_val;
  output_val = constrain((int(33 * current_mA)-3), 0, 1024);             // Value is 800/20 -> DACmax/mA max
  analogWrite(signal_out_pin, output_val);
  Serial.print(", output_val: ");
  Serial.print(output_val);
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  Serial.print("miss-signal -> starting");
  delay(2000);  // this makes sure program starts even if no connection
  oled_begin();
  // Set analog write to 10 bits to activate DAC output on pin A0
  //last_pot_read = analogRead(pot_pin);  // this makes it not auto update at start so current will display 0 and output 0 until control is moved
  analogWriteResolution(10);
  pinMode(signal_out_pin, OUTPUT);
  update_oled();
}

void loop() {
  read_pot();
  if (value_changed) {
    update_oled();
    output_current(current_output);
    value_changed = false;
  }
  //  delay(400);
}
