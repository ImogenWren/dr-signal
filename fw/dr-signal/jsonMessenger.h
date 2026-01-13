/* jsonMessenger.h

_Library to simplify two way communication over Serial interfaces using JSON formatted messages_

Imogen Heard
17/10/24

Please see: https://github.com/ImogenWren/jsonMessenger_library for latest version and usage instructions
NOTE: This library is memory intensive and may not be suitable for lightweight microcontrollers (uno, nano, AtTiny etc)

Version V1.0.0
- Focus on receiving & parsing incoming Serial CMD messages in JSON format




*/

//#pragma once

#ifndef jsonMessenger_h
#define jsonMessenger_h

#if (ARDUINO >= 100)
#include "Arduino.h"
#else
#include "WProgram.h"
#endif


#include <stdio.h>
#include <ArduinoJson.h>


#include "jsonConfig.h"



#define JSON_MESSENGER_VERSION "V2.0.0"
#define PRINT_RX_COMMAND true      // JSON safe printing of command received
#define PRINT_DESERIAL_ERROR true  // JSON safe printing of error status of deserialised command
#define DEBUG_JSON_MESSENGER true  // JSON safe printing of entire parsed message including ENUM values {"key":KEY,"num":KEY_ENUM,"dataType":DATATYPE_ENUM,"typeName":TYPENAME,"state":STATE_ENUM,"data":DATA}


/*
#if COMMAND_HINTS == true
char exampleCommands[][32] = {
  "{\"mode\":\"stop\"}",
  "{\"set\":\"mode\",\"to\":\"stop\"}",
  "{\"heater\":0}",
  "{\"heater\":1}",
  "{\"set\":\"heater\",\"val\":0}",
  "{\"flow\":0}",
  "{\"flow\":50}",
  "{\"set\":\"flow\",\"value\":75}",
};
#endif
*/





class jsonMessenger {


public:
  // constructor
  jsonMessenger();

  void jsonBegin();

  jsonStateData jsonReadSerialLoop();  // function needs to be called periodically to accept incoming serial messages, parse & return jsonStateData with new state /and data required for new state

  void printJSON(StaticJsonDocument<JSON_RX_SIZE> *jsonDoc);  // Prints the static jsonRXdoc


  const char *getCMDkey(jsonStates state);  // to print the state in human readable form without coping the entire list of keys to global scope

  const char *getDataType(dataTypes type);

  void printJSONdata(jsonStateData *data);

#if JSON_USE_QUEUE == true
  int8_t enque_cmd(jsonStateData *newItem);  // Returns the number of items in the queue, or -1 on fail

  jsonStateData dequeue_cmd();  // returns first queue item or 0 if queue is empty when called
#endif

private:



 // const authStruct blankAuth;   not needed

#if JSON_USE_QUEUE == true
  jsonStateData jsonQueue[CMD_QUEUE_LENGTH];
  int8_t queue_size = 0;  // must be managed "manually"
#endif
};



#endif
