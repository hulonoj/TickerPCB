#include <Arduino.h>
#include <string.h>
#include <iostream>
#include "Wire.h"
#include "ESP8266WiFi.h"
#include "networkSettings.h"
#include "gpioExpander.h"
#include "max6955.h"
#include "stateMachine.h"

// ------------------------
// Global Variables
// ------------------------

volatile byte buttons_valid = 0xFF;
volatile byte buttons = 0;
volatile byte prev_buttons = 0;

volatile uint8 GLOBAL_STATE = 0;
volatile uint8 GLOBAL_LOCAL_STATE = 0;

volatile bool GLOBAL_ENTER_HIT = false;
volatile bool GLOBAL_BUTTONS_UPDATED = false;
volatile bool GLOBAL_BUTTONS_DATA[8] = {false};

bool GLOBAL_WIFI_CONNECTED = false;

// ------------------------
// Interrupt Handler
// ------------------------

IRAM_ATTR void buttonPoll() {
  buttons = readGPIOExpander();

  if (buttons > prev_buttons) {
    GLOBAL_BUTTONS_UPDATED = true;

    for (int i = 0; i < 8; i++) {
      byte mask = 1 << i;
      if ((buttons & mask) && (buttons_valid & mask)) {
        buttons_valid &= ~mask;
        GLOBAL_BUTTONS_DATA[i] = true;

        if (i == 6) { // Mode button
          GLOBAL_STATE++;
          GLOBAL_LOCAL_STATE = 0;
        } else if (i == 7) { // Enter button
          GLOBAL_ENTER_HIT = true;
        }
      }
    }

  } else if (buttons < prev_buttons) {
    byte mask = 0x01;
    for (int i = 0; i < 8; i++) {
      if (!(buttons & mask) && (prev_buttons & mask)) {
        buttons_valid |= mask; // mark button as released
      }
      if (!(buttons & mask)) {
        GLOBAL_BUTTONS_DATA[i] = false; // reset pressed state
      }
      mask <<= 1;
    }
  }

  prev_buttons = buttons;
}

// ------------------------
// Setup & Loop
// ------------------------

void setup() {
  Serial.begin(115200);
  Serial.println("Setup start");

  Wire.begin(4, 5); // SDA, SCL

  attachInterrupt(digitalPinToInterrupt(12), buttonPoll, FALLING);

  initMAX6955();

  // setup_Wifi(); // Uncomment if needed

  prev_buttons = readGPIOExpander();
  writeDisplay("---HJ---");
  delay(1000);
}

void loop() {
  state_machine();
}
