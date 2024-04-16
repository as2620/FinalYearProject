#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_CCS811.h"
#include <Wire.h>

Adafruit_CCS811 ccs;

// ESP32 I2C pins
// GPIO 22 (SCL)
// GPIO 21 (SDA)

void setup() {
  Serial.begin(115200);

  Serial.println("CCS811 test");
  
  // Enable I2C
  Wire.begin(); 

  if(!ccs.begin()){
    Serial.println("Failed to start sensor! Please check your wiring.");
    while(1);
  }

  // Set the measurement mode
  ccs.setDriveMode(CCS811_DRIVE_MODE_250MS);

  // Wait for the sensor to be ready
  while(!ccs.available());
}

void loop() {
  // Serial.println("loop\n");
  if(ccs.available()){
    if(!ccs.readData()){
      // Print timestamp
      Serial.print(esp_timer_get_time());
      Serial.print(" ");
      // Print CO2 value
      Serial.print(ccs.geteCO2());
      Serial.print(" ");  
      // Print TVOC value
      Serial.println(ccs.getTVOC());
    }
    else{
      Serial.println("ERROR!");
      while(1);
    }
  }
}