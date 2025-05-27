#ifndef MCP9808_H
#define MCP9808_H

#include <Arduino.h>


// Temperature sensor MCP9808
// There are 2

float readMCP9808(int sensorNumber);
float readAndCalculateAverageTemp(bool celcius);

#endif