

#include "jsonMessenger.h"

/*

VERSION V0.2.0 REFACTORED TO USE ARRAYs instead of std::map


*/


jsonMessenger::jsonMessenger() {
}


void jsonMessenger::jsonBegin() {
  Serial.begin(115200);
  Serial.print(F("{\"json\":\"messenger\",\"version\":\""));
  Serial.print(JSON_MESSENGER_VERSION);
  Serial.println(F("\"}"));
}




void jsonMessenger::printJSON(StaticJsonDocument<JSON_RX_SIZE> *jsonDoc) {
#if PRETTY_PRINT_JSON == true
#pragma JSON set to Pretty Print !-May experience problems with Machine Parsing
  serializeJsonPretty(*jsonDoc, Serial);
#else
#pragma JSON set to standard JSON output - For human readability try enabling Pretty Print !
  serializeJson(*jsonDoc, Serial);
#endif
  Serial.print(F("\n"));
}





jsonStateData jsonMessenger::jsonReadSerialLoop() {

  jsonStateData jsonRX_data = { NONE, EMPTY, 0, 0.0, "", false };  // Default message that will be passed out if no data received or cannot be parsed

  if (Serial.available() > 0) {
    //Serial.println("Serial Available");
    char command[JSON_RX_SIZE];
    // create JSON document
    StaticJsonDocument<JSON_RX_SIZE> jsonRXdoc;
    // Receive Command
    Serial.readBytesUntil(10, command, JSON_RX_SIZE);  // 10 = "\n"

    for (int i = 0; i < JSON_RX_SIZE; i++) {  // trim command down to remove redundent characters
      if (command[i] == '}') {
        command[i + 1] = '\0';
        break;
      }
    }

#if PRINT_RX_COMMAND == true
    Serial.print(F("\n{\"rxed\": \""));
    Serial.print(command);
    Serial.println(F("\"}"));
#endif

    //Serial.read();  // clear any additional data left in the buffer

    // NEW FUNCTION, try deserializing directly from stream
    // deserializeJson(jsonRXdoc, Serial);  // less overhead but harder to debug
    DeserializationError error = deserializeJson(jsonRXdoc, command);  // more overhead but can print message before processing (good for debugging)

#if PRINT_DESERIAL_ERROR == true
    Serial.print("{\"deserialization\":\"");
    Serial.print(error.c_str());
    Serial.println(F("\"}"));
#endif
    JsonObject root = jsonRXdoc.as<JsonObject>();  // this was previously doc.to<JsonObject>(); DID NOT WORK! does now with "as"
                                                   // Now to parse the JSON message
                                                   // First get number of elements in the jsonStates enum numValues = NUM_VALUES;
                                                   // Then loop through all keys & see if root contains key

    bool set_keyword_used = false;  // Make the assumption that set keyword has not been used,

    const char *keyString;  // If the root contains "set" -> preload value (which contains key) into valueString and compare this along with root.key in loop below (2 birds, 1 stone)
    if (root.containsKey("set")) {
      keyString = jsonRXdoc["set"];
      set_keyword_used = true;  // if set has been used, then we need to extract the data in a slightly different way
    }

    for (int i = 0; i < NUM_VALUES; i++) {
      //  Serial.print(i);  // loop through all the keys
      //  Serial.print("  Checking against key: ");
      //  Serial.println(jsonCommandKeys[i]);

      if (root.containsKey(jsonCommandKeys[i]) || strcmp(keyString, jsonCommandKeys[i]) == 0) {  // Match is found, i holds the correct ENUM reference for the state

        jsonRX_data.data_type = dataTypes_array[jsonStateMap[i][1]];  // this should contain the correct enum for the datatype

#if DEBUG_JSON_MESSENGER == true
        Serial.print(F("{\"key\":\""));
        Serial.print(jsonCommandKeys[i]);
        Serial.print(F("\",\"num\":\""));
        Serial.print(i);
        Serial.print(F("\",\"dataType\":\""));
        Serial.print(jsonStateMap[i][1]);
        Serial.print("\",\"typeName\":\"");
        Serial.print(typeNames[jsonStateMap[i][1]]);
        Serial.print("\",");
        // Set the flags to trigger the state change
        Serial.print("\"state\":\"");
        Serial.print(jsonStateMap[i][0]);
        Serial.print("\",");
        Serial.print(F("\"data\":\""));
#endif


        //  jsonRX_data.cmdState = i;  //
        jsonRX_data.cmdState = jsonStates(jsonStateMap[i][0]);  // these two lines should be identical, except one passes the ENUM, the other (the same) int value
        jsonRX_data.cmd_received = true;                        //Set flag to be passed outside of library and notify a command has been sent (depreciated if using queue)



        // Copy data into  correct place in data structure, and convert to string for debugging printing
        // then deal with data depending on state


        if (jsonRX_data.data_type == EMPTY) {
          // Do nothing
        } else if (jsonRX_data.data_type == INTEGER) {  // Example of how to deal with different datatypes returned from jsonMessenger object
          if (set_keyword_used) {
            jsonRX_data.numeric = jsonRXdoc["to"].as<int16_t>();
          } else {
            jsonRX_data.numeric = jsonRXdoc[jsonCommandKeys[i]].as<int16_t>();
          }
#if DEBUG_JSON_MESSENGER == true
          Serial.print(jsonRX_data.numeric);
#endif
        } else if (jsonRX_data.data_type == FLOAT) {
          if (set_keyword_used) {
            jsonRX_data.floatData = root["to"].as<float>();
          } else {
            // jsonRX_data.floatData = jsonRXdoc[jsonCommandKeys[i]].as<float>();   // this line not working and stalling everything ( may have been unrelated!)
            jsonRX_data.floatData = root[jsonCommandKeys[i]].as<float>();  // try extracted from root doc instead
          }
#if DEBUG_JSON_MESSENGER == true
          Serial.print(jsonRX_data.floatData);
#endif
        } else if (jsonRX_data.data_type == CSTRING) {
          const char *extracted;
          if (set_keyword_used) {
            extracted = jsonRXdoc["to"].as<const char *>();
          } else {
            extracted = jsonRXdoc[jsonCommandKeys[i]].as<const char *>();
          }
          memcpy(jsonRX_data.msg, extracted, JSON_MSG_LENGTH);
          jsonRX_data.msg[JSON_MSG_LENGTH - 1] = '\0';
#if DEBUG_JSON_MESSENGER == true
          Serial.print(jsonRX_data.msg);
#endif
        } else if (jsonRX_data.data_type == AUTH) {  // if auth then two datas must be extracted
          const char *extract_secret;
          if (set_keyword_used) {
            jsonRX_data.numeric = jsonRXdoc["to"].as<int16_t>();
          } else {
            jsonRX_data.numeric = jsonRXdoc[jsonCommandKeys[i]].as<int16_t>();
          }
          extract_secret = jsonRXdoc["auth"].as<const char *>();  // this will always use auth keyword
          memcpy(jsonRX_data.msg, extract_secret, JSON_MSG_LENGTH);
          jsonRX_data.msg[8] = '\0';

#if DEBUG_JSON_MESSENGER == true
          Serial.print("data: ");
          Serial.print(jsonRX_data.numeric);
          Serial.print(" auth: ");
          Serial.print(jsonRX_data.msg);
#endif
        } else {
#if DEBUG_JSON_MESSENGER == true
          Serial.println(F("dataType-exception"));
#endif
        }
#if DEBUG_JSON_MESSENGER == true
        Serial.println("\"}");
#endif
#if JSON_USE_QUEUE == true
        if (jsonMessenger::enque_cmd(&jsonRX_data) == -1) {
          Serial.println(F("{\cmd\":\"enque-failed\"}"));
        }
#endif
        return jsonRX_data;  // return the structure as the data has been extracted
      } else {
        if (i == NUM_VALUES - 1) {  // if i = NUM_VALUES we have reached the end of the for loop, if no match has been found, print the unknown cmd
          Serial.println(F("{\"cmd\":\"unknown\"}"));
        }
        // We have pre-parsed the alternative set command so now else doesnt need to do anything
      }
    }
  }
  return jsonRX_data;
}



const char *jsonMessenger::getCMDkey(jsonStates state) {
  //std::cout << jsonCommandKeys[state] << "\0";
  return jsonCommandKeys[state];
}

const char *jsonMessenger::getDataType(dataTypes type) {
  //std::cout << typeNames[type] << "\0";
  return typeNames[type];
}

void jsonMessenger::printJSONdata(jsonStateData *data) {  // Use -> to assess members of a pointer to a struct
  Serial.print(F("cmdState: "));
  Serial.print(jsonMessenger::getCMDkey(data->cmdState));
  Serial.print(F(" dataType: "));
  jsonMessenger::getDataType(data->data_type);
  Serial.print(F(" Numeric: "));
  Serial.print(data->numeric);
  Serial.print(F(" Float: "));
  Serial.print(data->floatData);
  Serial.print(F(" msg: "));
  Serial.print(data->msg);
  Serial.print(F(" cmd: "));
  Serial.print(data->cmd_received);
  Serial.println();
}

#if JSON_USE_QUEUE == true
// Returns the number of items in the queue, or -1 on fail
int8_t jsonMessenger::enque_cmd(jsonStateData *newItem) {
  Serial.print(F("enque -> length:  "));
  if (queue_size < CMD_QUEUE_LENGTH) {
    jsonQueue[queue_size] = *newItem;
    queue_size++;
    Serial.println(queue_size);
    return queue_size;
  } else {
    Serial.println(-1);
    return -1;
  }
}
#endif


#if JSON_USE_QUEUE == true
// returns first queue item or 0 if queue is empty when called
jsonStateData jsonMessenger::dequeue_cmd() {
  jsonStateData first_item = { NONE, EMPTY, 0, 0.0, "", false };
  if (queue_size > 0) {
    jsonStateData first_item = jsonQueue[0];

    for (int i = 0; i < queue_size; i++) {
      //std::cout << "i: " << i << " i+1: " << i + 1 << std::endl;
      jsonQueue[i] = jsonQueue[i + 1];
    }
    queue_size--;
    Serial.print(F("Dequeue -> length:  "));
    Serial.println(queue_size);
    return first_item;
  } else {
    return first_item;
  }
}
#endif