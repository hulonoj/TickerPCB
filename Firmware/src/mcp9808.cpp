#include <Arduino.h>
#include "Wire.h"

#define MCP9808_I2C_ADDR_1 0x18
#define MCP9808_I2C_ADDR_2 0x19

#define MCP9808_CONFIG_REG 0x01 //0000 0001
#define MCP9808_TEMPERATURE_REG 0X05 //0000 0101
#define MCP9808_RESOLUTION_REG 0x08 //0000 1000

typedef union{
    uint16_t temp_bits;
    uint8_t reg_bits[2];
}mcp9808_temperature_reg;

// {2^(-4), 2^(-3), ... 2^6, 2^7 };
float mcp9808_temp_LUT[12]={
    0.0625, 0.125, 0.25, 0.5, 1.0, 2.0, 4.0, 8.0, 16.0, 32.0, 64.0, 128.0
};

float readMCP9808(int sensorNumber){

    mcp9808_temperature_reg temp_reg;
    float temperature = 0;

    uint16_t temp_i2c_address = sensorNumber == 0 ? MCP9808_I2C_ADDR_1 : MCP9808_I2C_ADDR_2;

    Wire.beginTransmission(temp_i2c_address);
    Wire.write(MCP9808_TEMPERATURE_REG);
    Wire.endTransmission();
    
    //request 2 byte from address (reg)
    Wire.requestFrom(temp_i2c_address, 2);

    //wait for response
    while(Wire.available() == 0);
    //put result in variables
    temp_reg.reg_bits[1] = Wire.read();
    temp_reg.reg_bits[0] = Wire.read();

    //mask out first 3 bits: not needed
    temp_reg.reg_bits[1] = temp_reg.reg_bits[1] & 0x1F;

    for(unsigned int i = 0; i < 12; i++){
        // for each bit, add the corresponding value in the LUT if needed
        if((temp_reg.temp_bits >> i) & 0x01){
            temperature += mcp9808_temp_LUT[i];
        }
    }

    if(temp_reg.reg_bits[1] & 0x10) { // 0001 0000
        // Negative Temp
        temperature = temperature * -1;
    }
    
    return temperature;
}

float readAndCalculateAverageTemp(bool celcius){
    float temperature0 = readMCP9808(0);
    float temperature1 = readMCP9808(1);
    if(celcius){
        return (temperature0 + temperature1)/2.0;
    }

    return ((temperature0 + temperature1)/2 * 9.0/5.0) + 32;
}   