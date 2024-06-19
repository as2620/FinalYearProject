//------------------------------------------------------------------------------------------------

// Readout Code for One CCS811 Sensor Connected to an ESP32.
// The sensor is read every 250ms and has a sampling rate of 4Hz.

//------------------------------------------------------------------------------------------------

#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_CCS811.h"
#include <Wire.h>

//------------------------------------------------------------------------------------------------

// Note: The CCS811 sensor should be connected to the default I2C pins. Otherwise the following 
// lines should be adjusted and passed into the begin() function of the Adafruit_CCS811 object
// and the Wire object.

// const uint8_t I2C_SCL = 22;
// const uint8_t I2C_SDA = 21;

// ------------------------------------------------------------------------------------------------

// CSS Sensor Object
Adafruit_CCS811 ccs;

// ------------------------------------------------------------------------------------------------

void setup() 
{
  Serial.begin(115200);

  Serial.println("CCS811 test");
  
  // Enable I2C
  Wire.begin(); 

  if(!ccs.begin())
  {
    Serial.println("Failed to start sensor! Please check your wiring.");
    while(1);
  }

  // Set the measurement mode to read every 250ms
  ccs.setDriveMode(CCS811_DRIVE_MODE_250MS);

  // Wait for the sensor to be ready
  while(!ccs.available());
}

//------------------------------------------------------------------------------------------------

void loop() 
{
  if(ccs.available())
  {
    if(!ccs.readData())
    {
      // Print timestamp
      Serial.print(esp_timer_get_time());
      Serial.print(" ");

      // Print CO2 value
      Serial.print(ccs.geteCO2());
      Serial.print(" ");  

      // Print TVOC value
      Serial.println(ccs.getTVOC());
    }
    else
    {
      Serial.println("ERROR: Failed to read data from sensor");
      while(1);
    }
  }
}

//------------------------------------------------------------------------------------------------
