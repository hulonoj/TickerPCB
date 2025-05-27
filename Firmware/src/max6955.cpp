#include <Arduino.h>
#include "Wire.h"

#define MAX6955_I2C_ADDR 0x60 
#define NUM_DISPLAYS 8

#define max6955_reg_decodeMode      0x01
#define max6955_reg_globalIntensity 0x02
#define max6955_reg_scanLimit       0x03
#define max6955_reg_configuration   0x04
#define max6955_reg_displayTest     0x07
#define max6955_reg_digitType       0x0C

void writeMAX6955(byte command, byte data)
{
    Wire.beginTransmission(MAX6955_I2C_ADDR);
    Wire.write(command);
    Wire.write(data);
    Wire.endTransmission();
}

void writeDisplay(String msg)
{
  byte j = 0;
  bool dotLit = false;
  for (byte i=0; i < 8; i++)
  {
    if(i +0x01 < (byte)msg.length() && msg[i+1] == '.'){
      dotLit = true;
    }
    
    if (i < msg.length())
      writeMAX6955(0x67-j, (dotLit? 0x80 : 0) | msg[i]);
    else
      writeMAX6955(0x67-j, ' ');
  
    if(dotLit){
      i++;
    }
    j++;
    dotLit = false;
  }
}

void clearDisplay(){
  writeDisplay("");
}
  

void writeChar(byte pos, char letter, boolean dotLit)
{
  writeMAX6955(0x60 + pos, (dotLit? 0x80 : 0) | letter);
  //writeMAX6955(0x61, (dotLit? 0x80 : 0) | letter);
}

void initMAX6955()
{

    //config reg, set normal operation
    writeMAX6955(max6955_reg_configuration, 0x01);

    // ascii decoding for all digits;
    writeMAX6955(max6955_reg_decodeMode, 0x0F);

    // brightness: 0x00 =  1/16 (min on)  2.5 mA/seg;
    // 0x0F = 15/16 (max on) 37.5 mA/segment
    writeMAX6955(max6955_reg_globalIntensity, 0x07);

    // active displays: 0x07 -> all;
    writeMAX6955(max6955_reg_scanLimit, 0x07);

    //digittype (14seg)
    writeMAX6955(max6955_reg_digitType, 0xFF);

}

void writeTime(int hours, int minutes, int seconds)
{
  //writeDisplay("");
  if(hours >= 13){
    hours = hours - 12;
  }
  char h1 = (hours/10)? '0' + hours/10 : ' ';
  writeChar(7, h1, false);
  char h2 = '0' + (hours%10);
  writeChar(6, h2, true);
  char m1 = '0' + (minutes/10);
  writeChar(4, m1, false);
  char m2 = '0' + (minutes%10);
  writeChar(3, m2, true);
  char s1 = '0' + (seconds/10);
  writeChar(1, s1, false);
  char s2 = '0' + (seconds%10);
  writeChar(0, s2, false);
}