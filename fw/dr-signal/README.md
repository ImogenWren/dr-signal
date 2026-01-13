# Ripple Tank Firmware
_Control a stepper motor and servo via JSON formatted command set. Gather data from position encoder and accellerometer mpu_

## Commands list
_List of basic commands in JSON format. This list is printed out on entry to STATE_WAIT and by entering STATE_HELP

_Verbose Command Structure_
```
   {"set":"start"}                   -> Start wave output to speaker
   {"set":"stop"}                    -> Stop wave output to speaker           
   {"set":"hz","to": 0 to 600 }      -> Set Wave Frequency
   {"set":"lamp","to": 0 or 1024 }   -> Set LED Brightness
   {"set":"amp","to": 0 to 255 }     -> Set wave amplitude
   {"set":"pulse","to": 0 to {uint32} } -> Pulse output for {_} mS
   {"set":"stream"}                  -> Start Data Streaming    
   {"set":"endst"}                   -> End Data Streaming      
   {"set":"snap"}                    -> Take Data Snapshot       
   {"set":"time","to": 1 - 250000 }  -> Set Time for Data Snapshot (mS)  
   {"set":"setcal", "to":"0 - 32768", "auth":"XXXXXXXX"} -> Set calibration offset"));              
   {"set":"help"}                    -> Print Commands to Serial Monitor 


```


_Succinct Command Structure_
- Follow the same structure as above, but does not require the "set" and "to" keywords. For singe word commands with no data passed, dummy value must be passed to ensure valid JSON structure eg:
- `{"start":"0"}` and `{"start":1}` are equivilent and will both function correctly
- `{"pulse":1000}` will pulse the wave output for 1 second
```

```





# jsonMessenger_library
_This library has been developed to simplify the development of embedded state machines with a JSON formatted command structure for sending commands and receiving data over a Serial interface_

It works by associating different keywords used to define commands sent by a user or user interface, with a list of known states, that could represent all, or some of the various states of a state machine, and 
a list of the expected variable types to be sent along with the defined keywords.



 ## Library Use
_The library is made up of several distinct parts_
1. `stateConfig.h` -> User Editable File: Defines the state machine running constantly on the embedded system. Every potential state should be described in this user editable file
2. `jsonConfig.h`  -> User Editable File: Defines the user commands, datatypes and all states that can be triggered via the jsonMessenger system, but does not need to define any states that are not linked with a Serial command
3. `jsonMessenger.h` -> jsonMessenger Library Header File, make sure to include this in `globals.h`. This file is not intended to be user editable, and can be placed in `libraries` along with `jsonMessenger.cpp`
4. `void loop() function` -> This function must call  `jsonMessenger::jsonReadSerialLoop();` and `sm_Run` and includes a small `if/if else` structure to tie jsonMessenger to the function of the state machine

Please Find detailed instruction on use and modification of the user editable files, and the code required inside the loop() function below.

## 1. Modifying `stateConfig.h`
_Defines the state machine running constantly on the embedded system. Every potential state should be described in this user editable file_ Note: This template can be used on its own without jsonMessenger if
the functions of the jsonMessenger libary are not required.


#### 1. Define all the valid states for the state machine with an enum
//    - This should include all states triggered by user input, but may also contain states that are only accessable programatically
```
typedef enum {
  STATE_INIT,
  STATE_WAIT,
  STATE_STOP,
  STATE_START,
  STATE_SET_SPEED_HZ,
  STATE_SET_SPEED_RPM,
  STATE_PING,
  STATE_HELP,
  NUM_STATES  // Sentinal value lets us get the total number of states without manually counting. Do not forget this value, it is important for correct function
} StateType;
```

#### 2. Define variables to hold the current, and previous state enum
Holding both these values allows us to compare them as we enter a state, to see if it is the first time we have entered the state.

```
StateType smState = STATE_INIT;  // This variable also defines the initial state
StateType lastState;
```

#### 3. Define a State Names Array
This will allow us to print the enum above in human readable format
```
char stateNames[][20] = {
  "STATE_INIT",
  "STATE_WAIT",
  "STATE_STOP",
  "STATE_START",
  "STATE_SET_SPEED_HZ",
  "STATE_SET_SPEED_RPM",
  "STATE_PING",
  "STATE_HELP"
};
```

#### 4. Define the state machine function prototypes
Any function that will be passed data from a user input will be passed the same jsonStateData structure as an argument
```
void sm_state_init(void);
void sm_state_wait(void);
void sm_state_stop(void);
void sm_state_start(void);
void sm_state_set_speed_hz(jsonStateData stateData);
void sm_state_set_speed_rpm(jsonStateData stateData);
void sm_state_ping(void);
void sm_state_help(void);
```




#### 5. Define all the state machine functions
 _A typical state function template is shown below_

```
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
```


#### 6. Finally define the state machine function `sm_Run()`
_A switch case structure calls each function based on the current `smState` variable_
Note -> Automatically generate the switch case from the list of ENUM states and list of function prototypes! -> https://github.com/ImogenWren/switch-case-generator

```
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
      case STATE_SET_SPEED_HZ:
        sm_state_set_speed_hz(stateData);
        break;
      case STATE_SET_SPEED_RPM:
        sm_state_set_speed_rpm(stateData);
        break;
      case STATE_PING:
        sm_state_ping();
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
    Serial.println(F("sm: Exception -> NUM_STATES exceeded"));
  }
}
```


## 2. Modifying `jsonConfig.h`
_This header should be used with the jsonMessenger library to define all the working states & commands that can be decoded by the jsonMessenger system._
This library is designed to parse commands recieved via the Serial object in Arduino. Commands should be formatted in CMD:VALUE pairs as follows:

#### Version 1 -> succinct command structure
`{"CMD":"VALUE"}` -> for CMDs with passed values  <br>
or <br>
`{"CMD":}` -> for CMDs with no additional values <br>
note: in the 2nd example, any data entered after `:` will be ignored, as we have already defined the datatypes that will be passed with each command to the parser.

#### Version 2 -> Verbose command structure
`{"set":"CMD","to":"VALUE"}` -> for CMDs with passed values  <br>
or  <br>
`{"set":"CMD"}` -> for CMDs with no additional data.  <br>

As the libary works via parsing JSON structures, any additional key:value pairs will simply be ignored, as long as the overal JSON structure is validated, the parser will just look
for the existance of matching keys, and ignore anything else. <br>

All modifications should be carried out in this header file `jsonConfig.h`, please do not modify jsonMessenger.h or jsonMessenger.cpp!


#### 1. Define an enum to define variable types
 These will be linked to a state enum so when a message with a keyword is received, we can look up what 
 data type will be included with the message
 
 ```
typedef enum {  // enum to pass variable types between functions
  EMPTY,
  INTEGER,
  FLOAT,
  CSTRING
} dataTypes;
```
1b. Place these enums into an array to enable lookup by index (This shouldnt be nessissary but it fixed a bug at one point)
`const dataTypes dataTypes_array[5] = { EMPTY, INTEGER, FLOAT, CSTRING, BOOL };` <br>

1c. Define char string for the above enum to allow us to print out the enum above in a human readable format!
```
static char typeNames[][12] = {
  "EMPTY",
  "INTEGER",
  "FLOAT",
  "CSTRING"
};
```

#### 2. Declare a list of all possible key values as ENUM. 
These values will be passed out of the jsonMessenger Object and can be used to control a state machine external to this library (i.e the one defined in stateConfig.h). <br>
_NOTE this list may not include all possible states, JUST the states that are triggered by receiving a command_ <br>
- Include a null value at 0 -> This is because this enum will be initialised at 0 to represent jsonMessenger not receiving data, or being unable to parse a command

```
typedef enum {
  NONE,                // Include null or none state
  STOP,
  START,
  SET_SPEED_HZ,
  SET_SPEED_RPM,
  PING,
  HELP,
  NUM_VALUES 
} jsonStates;
```

#### 3. Link each jsonState ENUM with the datatype ENUM in a ~~map~~<depreciated> structure -> Now uses 2D array!.
```
const uint16_t jsonStateMap[NUM_VALUES][2] = {
  { jsonStates::NONE, dataTypes::EMPTY },
  { jsonStates::STOP, dataTypes::EMPTY },
  { jsonStates::START, dataTypes::EMPTY },
  { jsonStates::SET_SPEED_HZ, dataTypes::FLOAT },
  { jsonStates::SET_SPEED_RPM, dataTypes::FLOAT },
  { jsonStates::PING, dataTypes::EMPTY },
  { jsonStates::HELP, dataTypes::EMPTY }
};



```

##### 4. Declare a list of key commands that will be required to be parsed 
This must match the order of the enums above, including the NULL or
NONE value, but does not have to include NUM_VALUES. This list will be the actual commands that can be parsed by the system
```
static char jsonCommandKeys[][5] = {
  "na",
  "stop",
  "start",
  "hz",
  "rpm",
  "ping",
  "help"
};
```
NOTE, this can also be used to turn the enums above back into strings for human readability. <br>


#### 5. Finally Declare a data structure
_This will hold both the jsonStates enum, and any data that will need to be passed from jsonMessenger and outside of this library._ <br>

We can make this fairly generic by including additional datatypes, or we can reduce the size of the memory used by removing the unneeded ones. <br>
This structure must include the `jsonState`, `dataType` and a `command_received` bool that can be used to flag that a new command has been received to methods 
outside of the library.  <br>

All other data is arbitary and will depend on use case, though the library has been built to accomidate those listed,
modification of the .cpp file may be needed to accomidate additional datatypes. 

```
struct jsonStateData {
  jsonStates cmdState;        // The command state enum to tell state machine what state to go to next
  dataTypes data_type;        // The type of data being passed along with structure (though state should know what data to expect anyway), this could be removed to save space
  int numeric;                // empty generic data slots for each data type
  float data;
  char msg[16];
  bool cmd_received;           // Flag set true by jsonLoop when cmd is received
};
```

<br>

## 3. Modifying code inside `void loop()` function & within Global Scope
_This is the point at which the `jsonMessenger` library and `stateConfig.h` meet. Carefully implementing this last step unlocks the full functionality of the header files configured in the previous steps_

#### 1. Create a global jsonMessenger Object
_I create this within the `globals.h` file, but feel free to put this anywhere within global scope_  <br>
`jsonMessenger jsonRX;` 

#### 2. Call `jsonMessenger::jsonReadSerialLoop()`
_This function listens for serial commands, parses them if received and returns a data structure including flags for the requested state, and the data passed into the system_  <br>
- If no serial command is received, or the data is unable to be parsed, it returns a null flag, and the program continues waiting for another command  <br>

Create a `jsonStateData` structure to receive the data from the loop function  <br>
`  jsonStateData nextState_data = jsonRX.jsonReadSerialLoop();`  <br>

#### 3. Check the structure for `cmd_recieved` flag 
 `if (nextState_data.cmd_received) {  // If command is received ` 

#### 4. If/If Else tree to parse the requested state enum, and define the state defined in `stateConfig.h`
```
 // This is the bit that parses the command recieved by user, and sets the state machine to go to the correct state
    if (nextState_data.cmdState == STOP) {  // if fan speed change command received
      smState = STATE_STOP;
    } else if (nextState_data.cmdState == START) {
      smState = STATE_START;
    } else if (nextState_data.cmdState == SET_SPEED_HZ) {
      smState = STATE_SET_SPEED_HZ;
    } else if (nextState_data.cmdState == SET_SPEED_RPM) {
      smState = STATE_SET_SPEED_RPM;
    } else if (nextState_data.cmdState == PING) {
      smState = STATE_PING;
    } else if (nextState_data.cmdState == HELP) {
      smState = STATE_HELP;
    } else {
      Serial.println("{\"WARNING\":\"Unrecognised cmdState\"}");
    }
  }
```
#### 5. Call the state machine function, and pass the data structure as an argument
`sm_Run(nextState_data);  // This Runs the state machine in the correct state, and is passed all of the data sent by the last command`


## Finished -> State Machine and jsonMessenger are now fully configured!
