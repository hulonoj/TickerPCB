#ifndef MAX6955_H
#define MAX6955_H

#include <Arduino.h>

void writeMAX6955(byte command, byte data);
void writeDisplay(String msg);
void clearDisplay();
void writeChar(byte pos, char letter, boolean dotLit);
void initMAX6955();
void writeTime(int hours, int minutes, int seconds);

#endif