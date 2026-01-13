
#pragma once

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <avr/dtostrf.h>  //needed for SAMD21 only


const int pot_pin = A7;
const int signal_out_pin = A0;
const int samples = 10;
const bool SHOW_SPASH_SCREEN = false;

float current_output = 0;

int last_pot_read = 0;

bool value_changed = false;


#include "oled.h"