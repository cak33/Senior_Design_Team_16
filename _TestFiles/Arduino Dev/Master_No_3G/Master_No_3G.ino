/* 
 * master.ino
 * 
 * This sketch is for master (gateway) in the GardeNet system. Functions include:
 *    - Establishing and maintaining a mesh network
 *    - Maintaining a 3G connection to the GardeNet server
 *    - Controlling up to 16 nodes
 *    - Tracking several variables with regards to its and the garden's status
 * 
 * (C) 2016, John Connell, Anthony Jin, Charles Kingston, and Kevin Kredit
 * Last Modified: 4/11/16
 */


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////  Preprocessor   ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// includes
#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <EEPROM.h>
#include <TimerOne.h>
//#include "C:/Users/Antonivs/Desktop/Arbeit/Undergrad/Senior_Design/repo/RadioWork/Shared/SharedDefinitions.h"
#include "C:/Users/kevin/Documents/Senior_Design_Team_16/RadioWork/Shared/SharedDefinitions.h"
#include "StandardCplusplus.h"
//#include <system_configuration.h>
//#include <unwind-cxx.h>
//#include <utility.h>
#include <Time.h>
//#include "C:/Users/Antonivs/Desktop/Arbeit/Undergrad/Senior_Design/repo/ScheduleClass/Schedule.h"
//#include "C:/Users/Antonivs/Desktop/Arbeit/Undergrad/Senior_Design/repo/ScheduleClass/Schedule.cpp"
//#include "C:/Users/Antonivs/Desktop/Arbeit/Undergrad/Senior_Design/repo/ScheduleClass/ScheduleEvent.h"
//#include "C:/Users/Antonivs/Desktop/Arbeit/Undergrad/Senior_Design/repo/ScheduleClass/ScheduleEvent.cpp"
#include "C:/Users/kevin/Documents/Senior_Design_Team_16/ScheduleClass/Schedule.h"
#include "C:/Users/kevin/Documents/Senior_Design_Team_16/ScheduleClass/Schedule.cpp"
#include "C:/Users/kevin/Documents/Senior_Design_Team_16/ScheduleClass/ScheduleEvent.h"
#include "C:/Users/kevin/Documents/Senior_Design_Team_16/ScheduleClass/ScheduleEvent.cpp"

// pins
//#define unused    2
#define BUTTON    3
#define LEDR      4
#define LEDG      5
//#define unused    6
#define RF24_CE   7
#define RF24_CS   8
//RF24_MOSI //predifined on ICSP header
//RF24_MISO //predifined on ICSP header
//RF24_SCK  //predifined on ICSP header
//#define unused    9
//#define unused    10
//#define unused    11
#define ThreeG    12
//#define unused    13
#define RESET_GND A0
#define RESET_PIN A1
//#define unused    A2
//#define unused    A3
//#define unused    A4
//#define unused    A5
//#define Modem_Serial Serial1


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////     Globals     ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// mesh network
RF24 radio(RF24_CE, RF24_CS);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

// 3G Modem 
String currentString = "";
volatile int incomingByte = 0;

// flags
volatile bool hadButtonPress = false;
volatile bool updateStatusFlag = false;

// other
Garden_Status gardenStatus;
Schedule weeklySchedule;
uint8_t statusCounter = 0;
time_t lastTime = 0;
time_t startTime = 0;


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////     ISRs        ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/* 
 * handleButtonISR()
 *
 * Interrupt service routine (ISR) called when the button is pressed; sets a flag and exits
 * 
 * @preconditions: button is tied to interrupt; button is pressed
 * @postconditions: hadButtonPress flag is set
 */ 
void handleButtonISR(){
  // gets rid of startup false positive by ignoring for a few seconds after startup
  if(statusCounter > 0){
    hadButtonPress = true; 
    Serial.println(F("\n[Detected buttonpress]"));
  }
}

/* 
 * updateStatusISR()
 * 
 * ISR called when timer1 times out; sets a flag and exits
 * 
 * @preconditions: timer interrupt must be enabled
 * @postconditions: updateNodeStatus flag is set
 */
void updateStatusISR(){
  updateStatusFlag = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////    3G Modem     ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/* 
 * setupModem()
 *  
 * This function sets up the 3G modem and ensures that it connects to the network
 * 
 * @preconditions: modem is not connected to the network
 * @postconditions: modem is connected to the network
*/
//void setupModem() {
//  Serial.println("Starting Cellular Modem");
//  // properly toggle 3G Modem I/O pin 
//  digitalWrite(ThreeG, LOW);
//  pinMode(ThreeG, OUTPUT);
//  digitalWrite(ThreeG, LOW);
//  delay(1100); // modem requires >1s pulse
//  // return I/O pin 12 to input/hi-Z state  
//  pinMode(ThreeG, INPUT);
//
//  // initialize serial port to communicate with modem
//  Modem_Serial.println("Initializing modem COM port...");
//  Modem_Serial.begin(115200);
//  while (!Modem_Serial);
//
//  // Soft reset of modem
//  Serial.println("Reseting modem");
//  Modem_Serial.println("ATZ");
//  delay(500);
//  while(PrintModemResponse() > 0);
//
//  // Connect to 3G cellular network
//  Serial.println("Waiting for network connection...");
//  boolean connectionGood = false;
//  while(!connectionGood){
//    Modem_Serial.println("AT+CREG?");
//    currentString = "";
//    delay(500);
//    while(Modem_Serial.available() > 0) {
//      getModemResponse();
//      // check currentString to see if network status is "0,1" or "0,5" which means we are connected
//      if((currentString.substring(currentString.length()-3, currentString.length()) == "0,1") || 
//        (currentString.substring(0, currentString.length()) == "0,5")) {
//        connectionGood = true;
//        Serial.println(); 
//        Serial.println("Connection successful!");
//        currentString = "";
//      }
//    }
//  }
//  while(PrintModemResponse() > 0);
//}

/* 
 * getModemIP()
 *  
 * This function gets an IP address for the 3G modem by resetting the modem's
 * PDP context once
 * @preconditions: the modem is connected to the 3G cell network, but does not have an IP
 * @postconditions: the modem retrieves its IP address or throws an error message if it
 * already has one
*/
//void getModemIP() {
//  //  setup TCP socket
//  Modem_Serial.println("AT#SCFG=1,1,0,0,600,2");
//  delay(500);
//  while(PrintModemResponse() > 0);
//
//  Modem_Serial.println("at#sgact=1,1");
//  delay(500);
//  while(PrintModemResponse() > 0); 
//  
//  // wait for 10s for the modem to retrieve IP address
//  delay(10000);
//  while(PrintModemResponse() > 0);  
//}

/* 
 * openSocket()
 *  
 * This function opens a TCP socket to the GardeNet server and send a message to the server
 * 
 * @preconditions: the modem is connected to the network and has retrieved its IP address
 * @postconditions: the modem is connected to the GardeNet server via a TCP socket
*/
//void openSocket() {
//  // initiate TCP socket dial
//  Modem_Serial.println("AT#SD=1,0,5530,\"gardenet.ddns.net\",0,0");
//  delay(500);
//  while(PrintModemResponse() > 0);
//}

/* 
 * disconnectModem()
 *  
 * This function disconnects the 3G modem from the network and reports the data usage from 
 * the previous session as well as total data usage through the serial port
 * 
 * @preconditions: the modem is connected to the 3G cellular network (its PDP context is on)
 * @postconditions: the modem is no longer connected to the 3G network (its PDP context is turned off)
*/
//void disconnectModem() {
//  Modem_Serial.println("AT#SGACT=1,0");
//  delay(250);
//  while(PrintModemResponse() > 0);
//  delay(2000);
//  
//  // monitor data usage since last reset: <cid>, <tot>, <sent>, <received>
//  // mode 0: reset data counter
//  // mode 1: report data from last session
//  // mode 2: report data from last reset (of the data counter) 
//  Modem_Serial.println("AT#GDATAVOL=1");
//  delay(250);
//  while(PrintModemResponse() > 0);
//  Modem_Serial.println("AT#GDATAVOL=2");
//  delay(250);
//  while(PrintModemResponse() > 0);
//}

/* 
 * PrintModemResponse()
 *  
 * This function prints the modem response and monitors the availability of
 * TX1/RX1, which are used by the modem
 * 
 * @param: none
 * @return: Modem_Serial.available() -- 1 if more bytes are still avaialble
 *  in the UART buffer, 0 if false
 * @preconditions: there might be incoming bytes buffered in the modem
 * @postconditions: the modem writes out the incoming byte(s) to the serial debug port
 *  and returns a 0 or 1
*/
//int PrintModemResponse() {
//  while(Modem_Serial.available() > 0) {
//    //read incoming byte from modem and write byte out to debug serial over USB
//    Serial.write(Modem_Serial.read());
//  } 
//  Serial.println("");
//  //return number of characters in modem response buffer -- should be zero, but some may have come in since last test
//  return Modem_Serial.available();
//}

/* 
 * getModemResponse()
 *  
 * This function prints the incoming data from the modem to the serial monitor
 * and builds a  string which is processed and then cleared whenever a new line is detected
 * 
 * @preconditions: there is a global string variable, currentString, waiting to be processed
 * @postconditions: currentString is concatenated with the incoming byte or cleared when a new
 *  line character is detected and the current message is not an exception
*/
//void getModemResponse() {
//  incomingByte = Modem_Serial.read();
//  if(incomingByte != -1) {
//    Serial.write(incomingByte); 
//  }
//  // NO CARRIER is an exception in which we need to reopen the socket
//  if(incomingByte == '\n' && currentString != "NO CARRIER") {
//    currentString = "";
//  } else if(currentString != "NO CARRIER") {
//    currentString += char(incomingByte);
//  } else {
//    currentString = currentString;
//  }
//}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////  JSON Parsing   ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// JSON format: 
// {"start_time" : "1.10", "stop_time" : "2.30", "day" : "Monday", "zone_ID" : "3"}
// New JSON format: 
// {"start_time" : "1.10", "stop_time" : "2.30", "day" : "Monday", "zone_ID" : "3", "valve_num" : "x" } ?

/* 
 * parseJSON()
 * 
 * This function examines the global string variable, currentString, and determines the corresponding action:
 * set up a new schedule, turn on the entire garden, or turn off the entire garden
 * 
 * @preconditions: currentString contains a JSON string or is true/false
 * @postconditions: a message is forwarded to the mesh network
*/
void parseJSON() {
  if ((currentString.substring(currentString.length()-18,currentString.length()-9) == "valve_num")) {
    Serial.println("");
    Serial.println("I got a scheduling event!");
    createEvent();
  } else if (currentString == "true") {
    Serial.println("");
    Serial.println("I need to turn on the full garden.");
  } else if (currentString == "false") {
    Serial.println("");
    Serial.println("I need to shut down the full garden.");
  }
}

/*
 * createEvent()
 * 
 * This function parse a JSON string and create a ScheduleEvent and insert it into the weekly schedule
 * 
 * @preconditions: currentString is a JSON string containing a schedule event
 * @postconditions: the schedule event is parsed and inserted into the weekly schedule
*/
void createEvent() {
  uint8_t myDay;
  int beginIdx = 0;
  ScheduleEvent tempEvent;
  // used to process start time and end time further
  // split the string by double quote
  int idx = currentString.indexOf("\"");
  char charBuffer[16];
  String arg;

  for(int i = 0; i <= 99; i++) {
    arg = currentString.substring(beginIdx, idx);
    arg.toCharArray(charBuffer, 16);
          
    // add error handling for atoi:
    // eventArray[i] = atoi(charBuffer);
    beginIdx = idx + 1;
    idx = currentString.indexOf("\"", beginIdx);
    // Serial.println(charBuffer);

    // only four pieces of information are needed
    if(i == 3) {
      char* separator = strchr(charBuffer, '.');
      *separator = 0;
      int myHour = atoi(charBuffer);
      separator = separator + 1;
      int myMin = atoi(separator);
      tempEvent.setStartHour(myHour);
      tempEvent.setStartMin(myMin);
    } else if (i == 7) {
      char* separator = strchr(charBuffer, '.');
      *separator = 0;
      uint8_t myHour = atoi(charBuffer);
      separator = separator + 1;
      uint8_t myMin = atoi(separator);
      tempEvent.setEndHour(myHour);
      tempEvent.setEndMin(myMin);
    } else if (i == 11) {
      myDay = dayDecoder(arg);
      Serial.print("This event is to be inserted into "); Serial.println(myDay);
    } else if (i == 15) {
      uint8_t myID = atoi(charBuffer);
      tempEvent.setNodeID(myID);
    } else if (i == 19) {
      uint8_t myValve = atoi(charBuffer);
      tempEvent.setValveNum(myValve);
      break;
    }
  }
  // insert this event to the weekly schedule
  weeklySchedule.insert(myDay, tempEvent);

}

/*
 * dayDecoder(String myDay)
 * 
 * This function translates the string name of a weekday into a number
 * 
 * @param: myDay, a string
 * @return: a number from 0 - 6 representing a weekday with 0 being Sunday
*/
int dayDecoder(String myDay) {
  if(myDay == "Monday") {
    return 1;
  } else if (myDay == "Tuesday") {
    return 2;
  } else if (myDay == "Wednesday") {
    return 3;
  } else if (myDay == "Thursday") {
    return 4;
  } else if (myDay == "Friday") {
    return 5;
  } else if (myDay == "Saturday") {
    return 6;
  } else if (myDay == "Sunday") {
    return 0;
  } else { 
    // then somethign is not right and we need to do something about it?
    return 11;
  }
}


/*
 * checkCurrentSchedule(int myDay)
 * 
 * This function checks all available schedule events on a specific weekday, pops them and print them out
 * to the serial debug port
 * 
 * @param: myDay, an integer representing a weekday
 * @preconditions: there are schedule events on a certain weekday in the weekly schedule
 * @postconditions: there is no events on a certain weekday in the weekly schedule
*/
//void checkCurrentSchedule(int myDay) {
//  while(weeklySchedule.isEmpty(myDay) == false){
//    ScheduleEvent myEvent = weeklySchedule.popFrontStartTime(myDay);
//    Serial.println("");
//    Serial.print("At node "); Serial.print(myEvent.getNodeID());
//    Serial.print(", valve "); Serial.print(myEvent.getValveNum()); Serial.print(" is set to have a start time of ");
//    Serial.print(myEvent.getStartHour()); Serial.print(":"); Serial.print(myEvent.getStartMin());
//    Serial.print(" and an end time of "); Serial.print(myEvent.getEndHour()); Serial.print(":"); Serial.print(myEvent.getEndMin());
//    Serial.println(".");
//  }
//}

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// Helper Functions///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


void initPins(){  
  // BUTTON
  pinMode(BUTTON, INPUT_PULLUP);
  // attach interrupt at the end of setup() in initPins2()
  
  // LEDR and LEDG
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);  

  // ThreeG
  // handle in modem-specific code; has very specific behavior

  // RESET_GND
  // initialize at the end of setup() in initPins2()

  // RESET_PIN
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, LOW);
}

void initPins2(){
  // attach interrupt to button
  attachInterrupt(digitalPinToInterrupt(BUTTON), handleButtonISR, FALLING);

  // enable self-resetting ability
  delay(50);  // allow capacitor to discharge if was previously charged before enabling autoreset again
              // 95% = 3*tau = 3*RC = 3*200*100*10^-6 = 60ms -- but never gets fully charged, and has
              //    been dicharging during previous setup, so 50ms is sufficient
  pinMode(RESET_PIN, INPUT);
  pinMode(RESET_GND, OUTPUT);
  digitalWrite(RESET_GND, LOW);
}

/* 
 * flashLED()
 * 
 * Flashes an LED based on the arguments
 * 
 * @preconditions: pins initialized
 * @postconditions: none
 * 
 * @param uint8_t whichLED: the pin name of the LED you want to flash, must be LEDR or LEDG
 * @param uint8_t myNum: the number of times you want LED to flash
 * @param uint8_t myTime: the duration of LED's on/off state when it flashes
*/
void flashLED(uint8_t whichLED, uint8_t myNum, uint8_t myTime){
  if(whichLED != LEDR && whichLED != LEDG) return;
  for (int i = 0; i < myNum; i++) {
    digitalWrite(whichLED, HIGH);
    delay(myTime);  
    digitalWrite(whichLED, LOW);
    delay(myTime);
  }  
}

/* 
 * hardReset()
 *
 * "Presses" the reset button by turning on the reset circuit
 * 
 * @preconditions: the pins are configured appropriately and the RESET_GND is set to LOW
 * @postconditions: the board resets
 */ 
void hardReset(){
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, HIGH);
  delay(1000);
  // arduino resets here
}

/* 
 * refreshReset()
 *
 * Discharges the capacitor in the resetting circuit. Must be called once every 5 minutes in order
 * to not reset the board.
 * 
 * @preconditions: pins configured
 * @postconditions: reset circuit is discharged, and will not reset for at least 5 minutes
 */ 
void refreshReset(){
  pinMode(RESET_PIN, OUTPUT);  
  digitalWrite(RESET_PIN, LOW);
  delay(50); // 95% charged: 3*tau = 3*RC = 3*200*100*10^-6 = 60 ms
  pinMode(RESET_PIN, INPUT);
}

void checkSchedule(){
  // for each node
  uint8_t node;
  for(node=1; node<=16; node++){
    // if registered
    if(gardenStatus.nodeStatusPtrs[node] != NULL){
      // if connected
      if(gardenStatus.nodeStatusPtrs[node]->meshState == MESH_CONNECTED){
//        Serial.print("checking node "); Serial.println(node);
        // for each valve
        uint8_t valve;
        for(valve=1; valve<=4; valve++){
//          Serial.print("checking valve "); Serial.print(valve);
          bool shouldBeOn;
          shouldBeOn = weeklySchedule.shouldValveBeOpen(weekday()-1, hour(), minute(), node, valve);
//          Serial.print(": should be "); shouldBeOn ? Serial.println("ON") : Serial.println("OFF");

//          Serial.print("And said valve is: "); Serial.println(gardenStatus.nodeStatusPtrs[node]->valveStates[valve].state);
          
          // if schedule says should be open and is closed
          if(shouldBeOn && gardenStatus.nodeStatusPtrs[node]->valveStates[valve].state == OFF){
            // send open signal
            Valve_Command vc;
            vc.whichValve = valve;
            vc.onOrOff = ON;
            vc.timeToLive = VALVE_COMMAND_TTL;
            safeMeshWrite(mesh.getAddress(node), &vc, SET_VALVE_H, sizeof(vc), DEFAULT_SEND_TRIES);
          }
          // else if schedule says should be closed and is open
          else if(shouldBeOn == false && gardenStatus.nodeStatusPtrs[node]->valveStates[valve].state == ON){
            // send close signal
            Valve_Command vc;
            vc.whichValve = valve;
            vc.onOrOff = OFF;
            vc.timeToLive = VALVE_COMMAND_TTL;
            safeMeshWrite(mesh.getAddress(node), &vc, SET_VALVE_H, sizeof(vc), DEFAULT_SEND_TRIES);
          }
          // else the state is as it should be
        }
      }
      // else node is not connected to mesh
      else{
        // TODO alert the server?
      }
    }
    // else unregistered
  }    
}


/* 
 * safeMeshWrite()
 *
 * Performs mesh.writes, but adds reliability features, hence "safe". If mesh.write doesn't work, 
 * then tries again after a set period of time; if a set number of tries doesn't work, then the 
 * addressed node is considered disconnected. Maximum latency = 5 seconds.
 * 
 * @preconditions: mesh is configured
 * @postconditions: message is sent, or else node is considered to be disconnected
 * 
 * @param uint8_t destination: the mesh address of the recipient
 * @param void* payload: the address of the data to send
 * @param char header: the message type that you are sending
 * @param uint8_t datasize: the size of the data to send, in bytes
 * @param uint8_t timesToTry: the number of remaining times to try to send
 * 
 * @return bool: true means send success, false means send failure
 */ 
bool safeMeshWrite(uint8_t destination, void* payload, char header, uint8_t datasize, uint8_t timesToTry){  
  // perform write
  if (!mesh.write(destination, payload, header, datasize)) {
    // if a write fails, check if have more tries
    if(timesToTry > 0){
      delay(RETRY_PERIOD);
      return safeMeshWrite(destination, payload, header, datasize, --timesToTry);
    }
    else{
      if(mesh.getNodeID(destination) != -1 && gardenStatus.nodeStatusPtrs[mesh.getNodeID(destination)] != NULL){
        gardenStatus.nodeStatusPtrs[mesh.getNodeID(destination)]->meshState = MESH_DISCONNECTED; // TODO is this the only way to tell?
        gardenStatus.numConnectedNodes--;
      }
      return false;
    }
  }
  else {
    // write succeeded
    Serial.print(F("Send of type ")); Serial.print(header); Serial.println(F(" success"));
    return true;
  }
}

bool checkNodeRegistered(uint8_t nodeID){
  if(gardenStatus.nodeStatusPtrs[nodeID] == NULL){
    gardenStatus.nodeStatusPtrs[nodeID] = new Node_Status;
    gardenStatus.numRegisteredNodes++;
    gardenStatus.numConnectedNodes++;
    return false;
  }
  return true;
}

void readMeshMessages(){
  mesh.update();
  mesh.DHCP();
  
  while(network.available()){
    RF24NetworkHeader header;
    network.peek(header);
    Serial.print(F("Received ")); Serial.print(char(header.type));
    Serial.print(F(" type message from node ")); Serial.println(mesh.getNodeID(header.from_node));

    switch(header.type){
    case SEND_VALVE_H:
      // A node is responding to a valve command. Read the response; if not what is expected, send
      //  another command to correct it; else all is well. (TODO add counter, time-to-live, so 
      //  don't have endless cycle in case things fail?

      // read in message
      Valve_Response vr;
      network.read(header, &vr, sizeof(vr));

      // parse results
      gardenStatus.nodeStatusPtrs[header.from_node]->isAwake = vr.nodeIsAwake;
      gardenStatus.nodeStatusPtrs[header.from_node]->valveStates[vr.whichValve].isConnected = vr.isConnected;
      gardenStatus.nodeStatusPtrs[header.from_node]->valveStates[vr.whichValve].state = vr.actualState;
      Serial.print("valve message from "); Serial.println(header.from_node);
      if(vr.nodeIsAwake == false){
        // then all valves are closed too
        gardenStatus.nodeStatusPtrs[header.from_node]->valveStates[1].state = OFF;
        gardenStatus.nodeStatusPtrs[header.from_node]->valveStates[2].state = OFF;
        gardenStatus.nodeStatusPtrs[header.from_node]->valveStates[3].state = OFF;
        gardenStatus.nodeStatusPtrs[header.from_node]->valveStates[4].state = OFF;
        // nothing else to be done
        break;
      }
      if(vr.actualState == NO_VALVE_ERROR){
        // do anything? already updated the data in the gardenStatus struct
      }
      else if(vr.timeToLive){
        // try again, if time to live hasn't run out
        Valve_Command vc;
        vc.whichValve = vr.whichValve;
        vc.onOrOff = vr.commandedOnOrOff;
        vc.timeToLive = vr.timeToLive - 1;
        safeMeshWrite(mesh.getAddress(header.from_node), &vc, SET_VALVE_H, sizeof(vc), DEFAULT_SEND_TRIES);
      }
      
      break;
    case SEND_NODE_STATUS_H:
      // A node is reporting its status. Update its status in gardenStatus

      // check that it's registered
      checkNodeRegistered(mesh.getNodeID(header.from_node));
      
      // read in the new status
      network.read(header, gardenStatus.nodeStatusPtrs[mesh.getNodeID(header.from_node)], sizeof(Node_Status));
      break;
    case SEND_JUST_RESET_H:
      // means that a node has recovered from reset; true means it was told to do so, false means a crash

      // check that it's registered
      checkNodeRegistered(mesh.getNodeID(header.from_node));

      bool toldToReset;      
      network.read(header, &toldToReset, sizeof(toldToReset));
      
      // TODO what to do with this information? ask for status update? -> will get this automatically...
      
      break;
    case SEND_NODE_SLEEP_H:
      // node had its button pressed and toggled sleep; update its status and TODO let the website know?

      // check that it's registered
      checkNodeRegistered(mesh.getNodeID(header.from_node));
      
      bool nodeIsAwake;
      network.read(header, &nodeIsAwake, sizeof(nodeIsAwake));

      gardenStatus.nodeStatusPtrs[mesh.getNodeID(header.from_node)]->isAwake = nodeIsAwake;

      // TODO what else to do with this information?
      
      break;
    case SEND_NEW_DAY_H:
      // node is responding that it got the "new day" message; should respond "true", meaning that it is awake
      
      // check that it's registered
      checkNodeRegistered(mesh.getNodeID(header.from_node));

      bool response;
      network.read(header, &response, sizeof(response));

      // TODO what else to do with this information?
      
      break;
    default:
      char placeholder;
      network.read(header, &placeholder, sizeof(placeholder));
      Serial.println(F("Unknown message type."));
      break;
    }
  }
}

/* 
 * initStatus()
 * 
 * Initializes the Garden_Status struct gardenStatus.
 * 
 * @preconditions: pins are configured
 * @postconditions: myStatus is gardenStatus
 */
void initGardenStatus(){  
  // isAwake
  gardenStatus.isAwake = true;
  
  // threeGState
  gardenStatus.threeGState = TR_G_DISCONNECTED;

  // meshState
  gardenStatus.meshState = MESH_NOT_BEGAN;

  // gardenState
  gardenStatus.gardenState = GARDEN_ALL_IS_WELL;

  // numRegisteredNodes
  gardenStatus.numRegisteredNodes = 0;

  // numConnectedNodes
  gardenStatus.numConnectedNodes = 0;

  // nodeStatusPrts -- initiate all to NULL
  *gardenStatus.nodeStatusPtrs = {0};
}

/* 
 * updateGardenStatus()
 *
 * Performs diagnostics on the 3G connection, mesh connection, and nodes
 * 
 * @preconditions: gardenStatus is initialized
 * @postconditions: gardenStatus is updated
 */
void updateGardenStatus(){

  //////////// CHECK 3G CONNECTION ////////////  

  // TODO
  

  //////////// CHECK MESH CONNECTION ////////////

  // assume began, so just check ratio of connected nodes
  if(gardenStatus.numConnectedNodes == gardenStatus.numRegisteredNodes){
    gardenStatus.meshState = MESH_ALL_NODES_GOOD;
  }
  else if(gardenStatus.numConnectedNodes == 0){
    gardenStatus.meshState = MESH_ALL_NODES_DOWN;
  }
  else{
    gardenStatus.meshState = MESH_SOME_NODES_DOWN;
  }
  
  //////////// CHECK NODE_STATUSES ////////////

  // manually check for unregistered nodes
  Serial.print("Num addresses in list: "); Serial.println(mesh.addrListTop);
  if(mesh.addrListTop-1 != gardenStatus.numRegisteredNodes){
    // if numbers don't match, check each registered address
    for(uint8_t i=1; i<mesh.addrListTop; i++){
      // if have a node registered in mesh but not in struct, request a status
      //    (will add it after the status is responded to)
      if(gardenStatus.nodeStatusPtrs[mesh.addrList[i].nodeID] == NULL){
        safeMeshWrite(mesh.addrList[i].address, '0', GET_NODE_STATUS_H, sizeof('0'), DEFAULT_SEND_TRIES);
      }
    }
  }

  gardenStatus.gardenState = GARDEN_ALL_IS_WELL;
  // cycle through CONNECTED nodes, and if any have an error, report
  uint8_t node;
  for(node=1; node<=16; node++){
    // if registered
    if(gardenStatus.nodeStatusPtrs[node] != NULL){
      // if connected
      if(gardenStatus.nodeStatusPtrs[node]->meshState == MESH_CONNECTED){
        // check if it has any errors; if so, report
        if(gardenStatus.nodeStatusPtrs[node]->voltageState != GOOD_VOLTAGE){
          gardenStatus.gardenState = GARDEN_NODE_ERROR;
          break;
        }
        else if(gardenStatus.nodeStatusPtrs[node]->flowState != HAS_NO_METER &&
                gardenStatus.nodeStatusPtrs[node]->flowState != NO_FLOW_GOOD &&
                gardenStatus.nodeStatusPtrs[node]->flowState != FLOWING_GOOD){
          gardenStatus.gardenState = GARDEN_NODE_ERROR;
          break;
        }
      }
    }
  }   
}

/* 
 * printGardenStatus()
 *
 * Prints gardenStatus to Serial port in a user-friendly way.
 * 
 * @preconditions: myStatus is initialized, Serial port is active
 * @postconditions: none
 */ 
void printGardenStatus(){
  // print number of times executed
  Serial.println(F("")); Serial.println(statusCounter++);

  // print time
  digitalClockDisplay();

  if(gardenStatus.isAwake == false) Serial.println(F("GARDEN IS IN STANDBY"));

  Serial.print(gardenStatus.numConnectedNodes); Serial.print(F("/"));
  Serial.print(gardenStatus.numRegisteredNodes); Serial.println(F(" nodes are connected"));

  Serial.print(F("3G status:      ")); 
  if(gardenStatus.threeGState == TR_G_CONNECTED){
    Serial.println(F("good (connected)"));
  }
  else if(gardenStatus.threeGState == TR_G_DISCONNECTED){
    Serial.println(F("DISCONNECTED"));
  }

  Serial.print(F("Mesh status:    "));
  if(gardenStatus.meshState == MESH_ALL_NODES_GOOD){
    Serial.println(F("good (all nodes connected)"));
  }
  else if(gardenStatus.meshState == MESH_NOT_BEGAN){
    Serial.println(F("not began"));
  }
  else if(gardenStatus.meshState == MESH_SOME_NODES_DOWN){
    Serial.println(F("some nodes down!"));
  }
  else if(gardenStatus.meshState == MESH_ALL_NODES_DOWN){
    Serial.println(F("ALL NODES DOWN!"));
  }

  Serial.print(F("Node statuses:  "));
  if(gardenStatus.gardenState == GARDEN_ALL_IS_WELL){
    Serial.println(F("good"));
  }
  else if(gardenStatus.meshState == GARDEN_NODE_ERROR){
    Serial.println(F("at least one connected node has an error!"));
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////     Timer       ////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/* This function prints out the current time recorded by the internal timer
 * in a readable way
 * @param: none
 * @return: none
*/
void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print(hour()); printDigits(minute()); printDigits(second());
  Serial.println();
  switch(weekday()-1){
    case 0:
      Serial.print(F("Sunday, "));
      break;
    case 1:
      Serial.print(F("Monday, "));
      break;
    case 2:
      Serial.print(F("Tuesday, "));
      break;
    case 3:
      Serial.print(F("Wednesday, "));
      break;
    case 4:
      Serial.print(F("Thursday, "));
      break;
    case 5:
      Serial.print(F("Friday, "));
      break;
    case 6:
      Serial.print(F("Saturday, "));
      break;
  }
  Serial.print(month()); Serial.print(F("/")); Serial.print(day());
  Serial.print(F("/")); Serial.println(year());
}

/* This function process the display of the minute and second of the internal 
 * timer
 * @param: digits, a digit between 0 and 60 which represents the current 
 *         minute or second recorded by the internal timer
 * @return: none
*/
void printDigits(int digits){
  // utility function for digital clock display: pruint8_ts preceding colon and leading 0
  Serial.print(F(":"));
  if(digits < 10)
    Serial.print(F("0"));
  Serial.print(digits);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////     Setup       ////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/* 
 * setup()
 *
 * The beginning of execution when the board boots; uses helper functions to initialize the pins, 
 * initialized myStatus, begin mesh communication, and more; see internal comments for more detail.
 * 
 * @preconditions: board just booted
 * @postconditions: all initializaion is complete, ready for normal operation
 */ 
void setup(){

  // initaialize all the pins
  initPins();

  // flash LEDR once to indicate the beginning of the setup process
  flashLED(LEDR, 1, 500);
  
  // begin serial communication
  Serial.begin(BAUD_RATE);
  while (!Serial);
  
  // check if button is being pressed; if so, do special startup
  if(digitalRead(BUTTON) == 0){
    flashLED(LEDG, 3, 125);

    // do special stuff
  }

  // initialize gardenStatus
  initGardenStatus();

  // TODO print status?

  // Setup 3G Modem
//  setupModem();
//  getModemIP();
//  openSocket();

  // Setup mesh
  mesh.setNodeID(MASTER_NODE_ID);
  while(!mesh.begin(COMM_CHANNEL, DATA_RATE, CONNECT_TIMEOUT)){
    Serial.println(F("Trouble setting up the mesh, trying again..."));
    delay(1000);
  }
  Serial.println(F("Mesh created"));
  mesh.setAddress(MASTER_NODE_ID, MASTER_ADDRESS);


  // init timer for regular system checks
  //Timer1.initialize(TIMER1_PERIOD);
  Timer1.initialize(1000000);
  Timer1.attachInterrupt(updateStatusISR);

  // setup time
  // Hard-coded time for testing purpose
  // setTime(hr,min,sec,day,mnth,yr)
  // remember to fix line 168 in Time.cpp
  setTime(15, 0, 0, 12, 4, 16);
  startTime = now();

  initPins2();

  // make my test schedule 
  currentString = "{\"start_time\" : \"14.50\", \"stop_time\" : \"15.10\", \"day\" : \"Tuesday\", \"zone_ID\" : \"1\", \"valve_num\" : \"1\" }";
  parseJSON();
  currentString = "{\"start_time\" : \"15.10\", \"stop_time\" : \"15.15\", \"day\" : \"Tuesday\", \"zone_ID\" : \"1\", \"valve_num\" : \"2\" }";
  parseJSON();
  currentString = "{\"start_time\" : \"15.11\", \"stop_time\" : \"15.12\", \"day\" : \"Tuesday\", \"zone_ID\" : \"1\", \"valve_num\" : \"3\" }";
  parseJSON();
  // print it
  checkCurrentSchedule(weekday()-1);
  // enter it again (was destroyed in printing)  
  currentString = "{\"start_time\" : \"14.50\", \"stop_time\" : \"15.10\", \"day\" : \"Tuesday\", \"zone_ID\" : \"1\", \"valve_num\" : \"1\" }";
  parseJSON();
  currentString = "{\"start_time\" : \"15.10\", \"stop_time\" : \"15.15\", \"day\" : \"Tuesday\", \"zone_ID\" : \"1\", \"valve_num\" : \"2\" }";
  parseJSON();
  currentString = "{\"start_time\" : \"15.11\", \"stop_time\" : \"15.12\", \"day\" : \"Tuesday\", \"zone_ID\" : \"1\", \"valve_num\" : \"3\" }";
  parseJSON();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////     Loop        ////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/* 
 * loop()
 *
 * Run indefinitely after setup() completes; contains the core node features. Uses helper functions
 * to control the reset circuit, maintain the mesh, update myStatus, handle buttonpresses, and 
 * handle communication with the master.
 * 
 * @preconditions: asetup() has successfully completed
 * @postconditions: none--runs forever
 */ 
void loop() {

  // speed up time
  if(second() > 0 && second() < 10 ){
    setTime(now()+55-second());
  }
  // and loop it to repeat the test schedule
  if(now() > startTime + 3600/2){
    setTime(startTime);
    lastTime = startTime;
  }

  refreshReset();

  // Communicate with server via 3G
//  while(Modem_Serial.available() > 0) {
//    getModemResponse();
//    parseJSON();
//  }
    
  // exceptions
//  if(currentString == "NO CARRIER" || currentString == "ERROR") {
//    openSocket();
//    currentString = "";
//  }
  
  // refresh the reset
  // refreshReset();

  // update node status if necessary
  if(updateStatusFlag){
    updateGardenStatus();
    printGardenStatus();
    
    // reset the flag
    updateStatusFlag = false;
  } 

  // check if need to open/close valves according to schedule
  // to occur at the beginning of each new minute
  if(now() >= lastTime + 60){
    if(gardenStatus.isAwake){
      checkSchedule();
    }
    lastTime = now();
  }

  // if had buttonpress, toggle between awake and asleep
  if(hadButtonPress){
    // toggle states between asleep and awake
    gardenStatus.isAwake = !gardenStatus.isAwake;

    // if asleep, tell valves to shut
    if(gardenStatus.isAwake == false){
      // TODO set light sequence
      
      // for each node, shut off all valves
      uint8_t node;
      for(node=1; node<=16; node++){
        // if registered
        if(gardenStatus.nodeStatusPtrs[node] != NULL){
          // if connected
          if(gardenStatus.nodeStatusPtrs[node]->meshState == MESH_CONNECTED){
            // send command to turn off all valves
            Valve_Command vc;
            vc.whichValve = ALL_VALVES;
            vc.onOrOff = OFF;
            vc.timeToLive = VALVE_COMMAND_TTL;
            safeMeshWrite(mesh.getAddress(node), &vc, SET_VALVE_H, sizeof(vc), DEFAULT_SEND_TRIES);
          }
        }
      }
    }
    else{
      // TODO set light sequence
    }

    // TODO report state to the server/website?
    
    // reset flag
    hadButtonPress = false;
  }


//  // for testing purpose only
//  if(currentString == "DONE") {
//    while(!weeklySchedule.isEmpty(1)) {
//      checkCurrentSchedule(1);
//    }
//  }

  // read in and respond to mesh messages
  readMeshMessages();
}

