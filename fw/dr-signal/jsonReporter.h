/*  jsonReporter.h

 Functions to load data into a json template and print to Serial Output

*/




#include "globals.h"

#include <avr/dtostrf.h>



// rounds a number to 2 decimal places
// example: round(3.14159) -> 3.14
double round2(double value) {
  return (int)(value * 100 + 0.5) / 100.0;
}


// rounds a number to X decimal places and ensures fixed width of signed and unsigned
void formatFloat(float value, char* buffer, size_t length, int decimalPlaces = 2) {
  if (length < 4) {  // Ensure buffer is large enough
    if (length > 0) buffer[0] = '\0';
    return;
  }


  char temp[16];  // Temporary buffer

  dtostrf(value, length - 1, decimalPlaces, temp);
  //Serial.println(F("ERRROR FORMAT FLOAT FUNCTION IS disabled at dtostrf ^^^^"));

  // Replace '+' with space for positive numbers
  if (temp[0] == '+') temp[0] = ' ';

  // Right-align by shifting if necessary
  size_t tempLen = strlen(temp);
  if (tempLen < length - 1) {
    size_t padding = length - 1 - tempLen;
    memmove(buffer + padding, temp, tempLen + 1);
    memset(buffer, ' ', padding);
  } else {
    strncpy(buffer, temp, length - 1);
    buffer[length - 1] = '\0';  // Ensure null termination
  }
}




void update_json(int16_t num_samples) {
  StaticJsonDocument<JSON_TX_BUFFER_SIZE> jsonTX;
  char float_buffer[10];  // buffer to holf fixed length floats
  //freeRAM = ram.getPrintStats("update_json");
  // Header
  jsonTX[F("timestamp")].set(millis());  //Message Timestamp set this last before printing
                                         // Payload
  jsonTX[F("payload")][F("state")].set(stateNames[smState]);


  // Output Data
  // Stepper Motor Settings
  //jsonTX[F("payload")][F("step")][F("hz")].set(step_hz);
  // jsonTX[F("payload")][F("step")][F("rpm")].set(step_rpm);
  // Encoder Data

  formatFloat(frequency, float_buffer, sizeof(float_buffer), 3);
  jsonTX[F("payload")][F("hz")].set(float_buffer);

  // formatFloat(stepper.encoder.getRPM(), float_buffer, sizeof(float_buffer), 3);
  //  jsonTX[F("payload")][F("encode")][F("rpm")].set(float_buffer);

  //  jsonTX[F("payload")][F("encode")][F("pos(raw)")].set(stepper.encoder.getAngleRaw());
  //  jsonTX[F("payload")][F("encode")][F("pos")].set(stepper.encoder.getAngle());

  // MPU Data
  // For single datapoints
  //  jsonTX[F("payload")][F("mpu")][F("acc")][F("x")].set(mpu.GetAccX());
  //  jsonTX[F("payload")][F("mpu")][F("acc")][F("y")].set(mpu.GetAccY());
  //  jsonTX[F("payload")][F("mpu")][F("acc")][F("z")].set(mpu.GetAccZ());
  //  jsonTX[F("payload")][F("mpu")][F("gyro")][F("x")].set(mpu.GetGyroX());
  //  jsonTX[F("payload")][F("mpu")][F("gyro")][F("y")].set(mpu.GetGyroY());
  //  jsonTX[F("payload")][F("mpu")][F("gyro")][F("z")].set(mpu.GetGyroZ());

  // For Arrays
  // create all the JSONarrays
  //JsonArray posArray = jsonTX[F("payload")][F("encode")].createNestedArray("pos");
  //JsonArray accXarray = jsonTX[F("payload")][F("mpu")][F("acc")].createNestedArray("x");
  //JsonArray accYarray = jsonTX[F("payload")][F("mpu")][F("acc")].createNestedArray("y");
  //JsonArray accZarray = jsonTX[F("payload")][F("mpu")][F("acc")].createNestedArray("z");
  //JsonArray gyroXarray = jsonTX[F("payload")][F("mpu")][F("gyro")].createNestedArray("x");
  //JsonArray gyroYarray = jsonTX[F("payload")][F("mpu")][F("gyro")].createNestedArray("y");
  //JsonArray gyroZarray = jsonTX[F("payload")][F("mpu")][F("gyro")].createNestedArray("z");

  jsonTX[F("payload")][("meta")][F("samples")].set(num_samples);
  JsonArray timeArray = jsonTX[F("payload")][F("meta")].createNestedArray("time");

  // for loop to place data into arrays (is it possible to just pass whole string without looping up to 200 times!?)

  for (int i = 0; i < num_samples; i++) {

    // Serial.print("i: ");
    // Serial.print(i);
    // Serial.print(", ");
    //  formatFloat(encode_array[i], float_buffer, sizeof(float_buffer), 3);
    ///  posArray.add(float_buffer);
    //   formatFloat(accX_array[i], float_buffer, sizeof(float_buffer), 3);
    //   accXarray.add(float_buffer);
    //   formatFloat(accY_array[i], float_buffer, sizeof(float_buffer), 3);
    //   accYarray.add(float_buffer);
    //   formatFloat(accZ_array[i], float_buffer, sizeof(float_buffer), 3);
    ///    accZarray.add(float_buffer);
    //   formatFloat(gyroX_array[i], float_buffer, sizeof(float_buffer), 3);
    //     gyroXarray.add(float_buffer);
    //     formatFloat(gyroY_array[i], float_buffer, sizeof(float_buffer), 3);
    //gyroYarray.add(float_buffer);//
    //     formatFloat(gyroZ_array[i], float_buffer, sizeof(float_buffer), 3);
    //gyroZarray.add(float_buffer);
    timeArray.add(timestamp_array[i]);
    //Serial.print(encode_array[i]);
    //  Serial.print(", ");
  }
  // Serial.println();



  // Meta Data & Debugging
  //jsonTX[F("meta")][F("sizeof")].set(sizeof(jsonTX));


  // int JSON_bytes = sizeof(jsonTX);  // used for debugging
  //Serial.print("Size of JSON: ");
  //Serial.println(JSON_bytes);

  if (PRINT_JSON) {
#if PRETTY_PRINT_JSON == true
#pragma JSON set to Pretty Print !-May experience problems with Machine Parsing
    serializeJsonPretty(jsonTX, Serial);
#else
#pragma JSON set to standard JSON output - For human readability try enabling Pretty Print !
    serializeJson(jsonTX, Serial);
#endif
    Serial.print(F("\n"));
  }
}