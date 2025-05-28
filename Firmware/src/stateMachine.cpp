#include <Arduino.h>
#include <max6955.h>
#include <networkSettings.h>
#include <mcp9808.h>
#include <string>

volatile extern bool GLOBAL_BUTTONS_DATA[8];
volatile extern uint8 GLOBAL_STATE;
volatile extern bool GLOBAL_BUTTONS_UPDATED;
volatile extern bool GLOBAL_ENTER_HIT;
volatile extern uint8 GLOBAL_LOCAL_STATE;
extern bool GLOBAL_WIFI_CONNECTED;

unsigned static long initTime = millis();
unsigned static long chessTime = millis();
static int ticker[4] = {0,0,0,0};
static long chessTimes[2] = {0,0};
static int chessIncrement = 0;
static bool chessSide = false;
static bool chessStart = false;

char alph_index(int &pos){

  if(pos >= 27){//if we go above or below the bound, reset
    pos = pos - 27;
  }
  if(pos < 0){
    pos = pos + 27;
  }

  if(pos == 0){
    return '_';
  }else{
    return ('A' - 1 + pos);
  }
}

void stock_selector(){
  if(!GLOBAL_BUTTONS_UPDATED){
    return;
  }
  GLOBAL_BUTTONS_UPDATED = false;
  if(GLOBAL_BUTTONS_DATA[0]){
    GLOBAL_BUTTONS_DATA[0] = false;
    if(GLOBAL_BUTTONS_DATA[4]){
      ticker[0] += 8;
    }else if(GLOBAL_BUTTONS_DATA[5]){
      ticker[0] --;
    }else{
      ticker[0]++;
    }
  }
  if(GLOBAL_BUTTONS_DATA[1]){
    GLOBAL_BUTTONS_DATA[1] = false;
    if(GLOBAL_BUTTONS_DATA[4]){
      ticker[1] += 8;
    }else if(GLOBAL_BUTTONS_DATA[5]){
      ticker[1] --;
    }else{
      ticker[1] ++;
    }
  }
  if(GLOBAL_BUTTONS_DATA[2]){
    GLOBAL_BUTTONS_DATA[2] = false;
    if(GLOBAL_BUTTONS_DATA[4]){
      ticker[2] += 8;
    }else if(GLOBAL_BUTTONS_DATA[5]){
      ticker[2] --;
    }else{
      ticker[2] ++;
    }
  }
  if(GLOBAL_BUTTONS_DATA[3]){
    GLOBAL_BUTTONS_DATA[3] = false;
    if(GLOBAL_BUTTONS_DATA[4]){
      ticker[3] += 8;
    }else if(GLOBAL_BUTTONS_DATA[5]){
      ticker[3] --;
    }else{
      ticker[3] ++;
    }
  }

  //Now update the display
  writeChar(0, alph_index(ticker[0]), false);
  writeChar(1, alph_index(ticker[1]), false);
  writeChar(2, alph_index(ticker[2]), false);
  writeChar(3, alph_index(ticker[3]), false);
}

void chess_selector(){
  if(GLOBAL_BUTTONS_DATA[0]){
    GLOBAL_BUTTONS_DATA[0] = false;
    writeDisplay("");
    writeDisplay("1 / 0");
    chessIncrement = 0;
    chessTimes[0] = 60;
    chessTimes[1] = 60;
  }else if(GLOBAL_BUTTONS_DATA[1]){
    GLOBAL_BUTTONS_DATA[1] = false;
    writeDisplay("");
    writeDisplay("5 / 0");
    chessIncrement = 0;
    chessTimes[0] = 300;
    chessTimes[1] = 300;
  }else if(GLOBAL_BUTTONS_DATA[2]){
    GLOBAL_BUTTONS_DATA[2] = false;
    writeDisplay("");
    writeDisplay("10 / 0");
    chessIncrement = 0;
    chessTimes[0] = 600;
    chessTimes[1] = 600;
  }else if(GLOBAL_BUTTONS_DATA[3]){
    GLOBAL_BUTTONS_DATA[3] = false;
    writeDisplay("");
    writeDisplay("5 / 5");
    chessIncrement = 5;
    chessTimes[0] = 300;
    chessTimes[1] = 300;
  }else if(GLOBAL_BUTTONS_DATA[4]){
    GLOBAL_BUTTONS_DATA[4] = false;
    writeDisplay("");
    writeDisplay ("10 / 5");
    chessIncrement = 5;
    chessTimes[0] = 600;
    chessTimes[1] = 600;
  }else if(GLOBAL_BUTTONS_DATA[5]){
    GLOBAL_BUTTONS_DATA[5] = false;
    writeDisplay("");
    writeDisplay("15 / 10");
    chessIncrement = 10;
    chessTimes[0] = 900;
    chessTimes[1] = 900;
  }
}

void chessTickTick(){
  if(chessTimes[0] == 0 && chessTimes[1] == 0){
    return;
  }
  if(chessTimes[0] >=  3600){
    chessTimes[0] = 3600;
  }
  if(chessTimes[1] >= 3600){
    chessTimes[1] = 3600;
  }
  if(chessTimes[0] <= 0){
    chessTimes[0] = 0;
    chessTimes[1] = 500;
    writeDisplay("P2 Wins");
    return;
  }
  if(chessTimes[1] <= 0){
    chessTimes[1] = 0;
    chessTimes[0] = 500;
    writeDisplay("P1 Wins");
    return;
  }
  int min1 = chessTimes[1] / 60;
  int min0 = chessTimes[0] / 60;

  int sec1 = chessTimes[1] % 60;
  int sec0 = chessTimes[0] % 60;
  
  //write time 1
  char m1 = '0' + (min1/10);
  if(m1 == '0'){
    writeChar(3, ' ', false);
  }else{
    writeChar(3, m1, false);
  }
  char m2 = '0' + (min1%10);
  writeChar(2, m2, true);
  char s1 = '0' + (sec1/10);
  writeChar(1, s1, false);
  char s2 = '0' + (sec1%10);
  writeChar(0, s2, false);

  //write time 0
 
  m1 = '0' + (min0/10);
  if(m1 == '0'){
    writeChar(7, ' ', false);
  }else{
    writeChar(7, m1, false);
  }
  m2 = '0' + (min0%10);
  writeChar(6, m2, true);
  s1 = '0' + (sec0/10);
  writeChar(5, s1, false);
  s2 = '0' + (sec0%10);
  writeChar(4, s2, false);
}

String ticker_to_string(){
  String ticker_out = "";
  if(alph_index(ticker[3]) != '_'){
    ticker_out += alph_index(ticker[3]);
  }
  if(alph_index(ticker[2]) != '_'){
    ticker_out += alph_index(ticker[2]);
  }
  if(alph_index(ticker[1]) != '_'){
    ticker_out += alph_index(ticker[1]);
  }
  if(alph_index(ticker[0]) != '_'){
    ticker_out += alph_index(ticker[0]);
  }
  ticker_out.toUpperCase();
  return ticker_out;
}

typedef void (*funcptr)(void);

// run_state(stete_function_pointer)

// each state function will be its own universe
// that functions with the few global variables that are shared

// Shared globals will be
// - byte buttons
// - mode hit
// - enter hit
// - buttons updated flag

void STATE_Stock(){
  // in here will be anothe state machine
  // state 1: wifi, unless already set up
  // STATE #1
  //static int (*localState) = 0;
  static bool wifi_has_been_set_up = false;

  volatile uint8* localState = &GLOBAL_LOCAL_STATE;

  switch(*localState){
  
    case 0: // Write Stonks
      clearDisplay();
      writeDisplay("Stonks");
      delay(100);
      (*localState)++;

    case 1: // Holding Zone
      if(!GLOBAL_ENTER_HIT){break;}
      GLOBAL_ENTER_HIT = false;
      (*localState)++;

    case 2: // WIFI SETUP STATE
      if(GLOBAL_WIFI_CONNECTED){
        (*localState)++;
        break;
      }

      //set up wifi
      if(!setup_Wifi()){
        delay(2000);
        (*localState)=0;
        break;
      }

      // Wifi is now set up, move on to next state.
      delay(1000);
      (*localState)++;
    case 3: // Run stock selector to pick stock ticker
      stock_selector();

      if(!GLOBAL_ENTER_HIT){break;}
      GLOBAL_ENTER_HIT = false;
      (*localState)++;
    case 4: // Display stock price
      writeDisplay(wifi_stock_price(ticker_to_string()));
      (*localState)++;
    case 5:
      if(!GLOBAL_ENTER_HIT){break;}
      GLOBAL_ENTER_HIT = false;
      // if we hit enter, go back to stock selector
      (*localState) = 2;
      clearDisplay();
      break;
    default:
      (*localState) = 0;
      break;
  }

  
}

void STATE_Temperature(){
  volatile uint8* localState = &GLOBAL_LOCAL_STATE;

  switch(*localState){
    case 0: // Landing in Temp, set and fall
      clearDisplay();
      writeDisplay("Temp");
      delay(100);
      (*localState)++;
    case 1: // Holding Zone
      if(!GLOBAL_ENTER_HIT){break;}
      GLOBAL_ENTER_HIT = false;
      (*localState)++;
    case 2:
      if(GLOBAL_ENTER_HIT){//move to next state
        GLOBAL_ENTER_HIT = false;
        (*localState)++;
        break;
      }

      // otherwise, display temp in C
      if(millis()-initTime > 1000){
        initTime = millis();
        writeDisplay(String(readAndCalculateAverageTemp(true), 4) + "C");
      }
      break;
    case 3:
      if(GLOBAL_ENTER_HIT){//move to previous state
          GLOBAL_ENTER_HIT = false;
          (*localState)--;
          break;
      }

      // otherwise, display temp in F
      if(millis()-initTime > 1000){
        initTime = millis();
        writeDisplay(String(readAndCalculateAverageTemp(false), 4) + "F");
      }
      break;
    default:
      (*localState) = 0;
      break;
    }
}

void STATE_ChessClock() {
  volatile uint8* localState = &GLOBAL_LOCAL_STATE;
  static bool readyToRun = false;
  static unsigned long lastTick = millis();
  static unsigned long lastDisplayUpdate = millis();

  // The first section of each state is the holding area from the last state.

  switch (*localState) {
    case 0: // Initial landing
      clearDisplay();
      writeDisplay("Chess");
      delay(100);
      (*localState)++;
      break;

    case 1: // Wait for enter to continue
      if(!GLOBAL_ENTER_HIT){break;}

      GLOBAL_ENTER_HIT = false;
      writeDisplay("Push 0-5");
      (*localState)++;
      break;

    case 2: // User selects time control
      if(GLOBAL_BUTTONS_UPDATED){
        GLOBAL_BUTTONS_UPDATED = false;
        chess_selector();  // handles button-based time mode selection
        readyToRun = true;
      }
      if (GLOBAL_ENTER_HIT && readyToRun) {
        GLOBAL_ENTER_HIT = false;
        chessStart = false;
        clearDisplay();
        chessTickTick();  // draw starting time
        (*localState)++;
      }
      break;

    case 3: // Chess clock running
      if (GLOBAL_BUTTONS_UPDATED){
        GLOBAL_BUTTONS_UPDATED = false;
        if (GLOBAL_BUTTONS_DATA[0] || GLOBAL_BUTTONS_DATA[1] || GLOBAL_BUTTONS_DATA[2]) {
          GLOBAL_BUTTONS_DATA[0] = false;
          GLOBAL_BUTTONS_DATA[1] = false;
          GLOBAL_BUTTONS_DATA[2] = false;
          
          chessSide = true;
          if (chessStart) {
            chessTimes[1] += chessIncrement;
          }
          chessStart = true; //dont increment on the first move
        }
        
        if (GLOBAL_BUTTONS_DATA[3] || GLOBAL_BUTTONS_DATA[4] || GLOBAL_BUTTONS_DATA[5]) {
          GLOBAL_BUTTONS_DATA[3] = false;
          GLOBAL_BUTTONS_DATA[4] = false;
          GLOBAL_BUTTONS_DATA[5] = false;
          
          chessSide = false;
          if (chessStart) {
            chessTimes[0] += chessIncrement;
          }
          chessStart = true;//dont increment on the first move
        }
      }

      // Tick time every second
      if (millis() - lastTick >= 1000 && chessStart) {
        lastTick = millis();
        if (chessSide) {
          if (chessTimes[0] > 0) chessTimes[0]--;
        } else {
          if (chessTimes[1] > 0) chessTimes[1]--;
        }
      }

      // Update display every ~333ms
      if (millis() - lastDisplayUpdate >= 333 && chessStart) {
        lastDisplayUpdate = millis();
        chessTickTick();
      }

      // Press enter to return to mode selection
      if (GLOBAL_ENTER_HIT) {
        GLOBAL_ENTER_HIT = false;
        (*localState) = 1;
        writeDisplay("Push 0-5");
        chessStart = false;
      }

      break;

    default:
      (*localState) = 0;
      break;
  }
}

void STATE_Clock() {
  volatile uint8* localState = &GLOBAL_LOCAL_STATE;
  static unsigned long lastLocalUpdate = 0;
  static unsigned long lastNtpSync = 0;
  static int hours = 0;
  static int minutes = 0;
  static int seconds = 0;
  static int timeZone = 0;
  static bool showColon = true;
  static bool militaryTime = true;

  switch (*localState) {
    case 0: // Startup
      clearDisplay();
      writeDisplay("Clock");
      delay(100);
      (*localState)++;
      break;

    case 1: // Holding Zone
      if(!GLOBAL_ENTER_HIT){break;}
      GLOBAL_ENTER_HIT = false;
      (*localState)++;

    case 2: // WIFI SETUP STATE
      if(GLOBAL_WIFI_CONNECTED){
        (*localState)++;
        break;
      }

      //set up wifi
      if(!setup_Wifi()){
        delay(2000);
        (*localState)=0;
        break;
      }

      // wifi is now enabled, move on to next state.
      delay(2000);
      (*localState)++;

    case 3:
      // Write "EST" to the displays
      timeZone = 0;
      lastNtpSync = 0; // set sync time to be outdated so it gets synced.
      clearDisplay();
      writeDisplay("EST");
      delay(1000);
      clearDisplay();
      (*localState)= 7;
      break;
    case 4:
      // Write "CST" to the displays
      timeZone = 1;
      lastNtpSync = 0; // set sync time to be outdated so it gets synced.
      clearDisplay();
      writeDisplay("CST");
      delay(1000);
      clearDisplay();
      (*localState) = 7;
      break;
    case 5:
      // Write "PST" to the displays
      timeZone = 2;
      lastNtpSync = 0; // set sync time to be outdated so it gets synced.
      clearDisplay();
      writeDisplay("MST");
      delay(1000);
      clearDisplay();
      (*localState) = 7;
      break;
    case 6:
      // Write "PST" to the displays
      timeZone = 3;
      lastNtpSync = 0; // set sync time to be outdated so it gets synced.
      clearDisplay();
      writeDisplay("PST");
      delay(1000);
      clearDisplay();
      (*localState) = 7;
      break;
    case 7:  // Display clock continuously

      // Sync time from NTP every 10 seconds
      if (millis() - lastNtpSync >= 10000 || lastNtpSync == 0) {
        lastNtpSync = millis();
        timeClient.update();
        hours = timeClient.getHours() - timeZone;
        minutes = timeClient.getMinutes();
        seconds = timeClient.getSeconds();
      }

      // Locally increment time every second
      if (millis() - lastLocalUpdate >= 1000) {
        lastLocalUpdate = millis();
        seconds++;

        if (seconds >= 60) {
          seconds = 0;
          minutes++;
        }
        if (minutes >= 60) {
          minutes = 0;
          hours++;
        }
        if (hours >= 24) {
          hours = 0;
        }
        if (hours < 0){
          hours = 24 + hours;
        }

        if(militaryTime){

          // Format: HH:MM:SS
          writeChar(7, '0' + (hours / 10), false);
          writeChar(6, '0' + (hours % 10), showColon); // colon after HH
          writeChar(5, '0' + (minutes / 10), false);
          writeChar(4, '0' + (minutes % 10), showColon); // colon after MM
          writeChar(3, '0' + (seconds / 10), false);
          writeChar(2, '0' + (seconds % 10), false);
          writeChar(1, ' ', false);
          writeChar(0, ' ', false);
          
          showColon = !showColon;
        }else{
          
          // Convert to 12-hour format
          bool isPM = false;
          int displayHour = hours;
          
          if (displayHour >= 12) isPM = true;
          if (displayHour == 0) displayHour = 12;
          else if (displayHour > 12) displayHour -= 12;
          
          // Format: HH:MM AM or PM
          writeChar(7, '0' + (displayHour / 10), false);            // Tens digit of hour
          writeChar(6, '0' + (displayHour % 10), showColon);        // Ones digit + colon
          writeChar(5, '0' + (minutes / 10), false);                // Tens digit of minutes
          writeChar(4, '0' + (minutes % 10), false);                // Ones digit of minutes
          writeChar(3, isPM ? 'P' : 'A', false);                    // 'P' or 'A'
          writeChar(2, 'M', false);                                 // 'M'
          writeChar(1, ' ', false);
          writeChar(0, ' ', false);
          
          showColon = !showColon;
        }
      }
      if (GLOBAL_ENTER_HIT) {
        GLOBAL_ENTER_HIT = false;
        switch(timeZone){
          case 0: (*localState) = 4; break;
          case 1: (*localState) = 5; break;
          case 2: (*localState) = 6; break;
          default: timeZone = 0; (*localState) = 3; break;
        }
        clearDisplay();
        delay(100);
      }
      if (GLOBAL_BUTTONS_UPDATED){
        for(int buttonNumber = 0; buttonNumber <= 5; buttonNumber++){
          if(GLOBAL_BUTTONS_DATA[buttonNumber]){
            // If we hit any button, toggle military time
            militaryTime = !militaryTime;
          }
        }
      }
      break;

    default:
      (*localState) = 0;
      timeZone = 0;
      break;
  }
}

funcptr StateFunctions[4] = {STATE_Clock, STATE_Temperature, STATE_Stock, STATE_ChessClock};


void state_machine(){

  switch(GLOBAL_STATE){
    case 0:
      STATE_Temperature();
      break;
    case 1:
      STATE_Stock();
      break;
    case 2:
      STATE_ChessClock();
      break;
    case 3:
      STATE_Clock();
      break;
    default:
      GLOBAL_STATE = 0;
      break;
  }
}