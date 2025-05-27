#include <Arduino.h>
#include <Wire.h>

#define GPIO_EXP_I2C_ADDR 0x20

#define gpio_input_reg 0x00
#define gpio_output_reg 0x01
#define gpio_polarity_inv_reg 0x02
#define gpio_config_reg 0x03

//public variables
volatile boolean dot = true;

byte readGPIOExpander()
{
  //begin the transmission
  Wire.beginTransmission(GPIO_EXP_I2C_ADDR);
  //send a bit and ask for register (reg)
  Wire.write(gpio_input_reg);
  //end transmission
  Wire.endTransmission();
  //request 1 byte from address (reg)
  Wire.requestFrom(GPIO_EXP_I2C_ADDR, 1);
  //wait for response
  while(Wire.available() == 0);
  //put result in variable buttons
  byte buttons = Wire.read();
  return buttons;
}