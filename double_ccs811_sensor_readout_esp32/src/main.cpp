#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_CCS811.h"
#include <Wire.h>

Adafruit_CCS811 ccs_1;
Adafruit_CCS811 ccs_2;

// ESP32 I2C pins
// First CSS Sensor
const uint8_t I2C_SDA_1 = 21;
const uint8_t I2C_SCL_1 = 22;

// Second CSS Sensor
const uint8_t I2C_SDA_2 = 33;
const uint8_t I2C_SCL_2 = 32;

// I2C Buses
TwoWire I2C_CCS_2 = TwoWire(0);

// Task Handles 
TaskHandle_t CCS_1_Task_Handle;
TaskHandle_t CCS_2_Task_Handle;


void CCS_1_Task(void *pvParameters)
{
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  if(!ccs_1.begin())
  {
    Serial.println("Failed to start sensor! Please check your wiring.");
    while(1);
  }

  ccs_1.setDriveMode(CCS811_DRIVE_MODE_250MS);
  while(!ccs_1.available());

  while(1)
  {
    if(ccs_1.available())
    {
      if(!ccs_1.readData())
      {
        // Print timestamp
        Serial.print("Sensor 1");
        Serial.print(" ");
        Serial.print(esp_timer_get_time());
        Serial.print(" ");
        // Print CO2 value
        Serial.print(ccs_1.geteCO2());
        Serial.print(" ");  
        // Print TVOC value
        Serial.println(ccs_1.getTVOC());
      }
    }
  }
}

void CCS_2_Task(void *pvParameters)
{
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());
  
  // (2) Intialise the second CCS sensor (using the second I2C bus
  if(!ccs_2.begin(0x5A, &I2C_CCS_2))
  {
    Serial.println("Failed to start sensor! Please check your wiring.");
    while(1);
  }

  // (3) Set the measurement mode
  ccs_2.setDriveMode(CCS811_DRIVE_MODE_250MS);
  
  // (4) Wait for the sensor to be ready
  while(!ccs_2.available());

  while(1)
  {
    if(ccs_2.available())
    {
      if (!ccs_2.readData())
      {
        // Print timestamp
        Serial.print("Sensor 2");
        Serial.print(" ");
        Serial.print(esp_timer_get_time());
        Serial.print(" ");
        // Print CO2 value
        Serial.print(ccs_2.geteCO2());
        Serial.print(" ");  
        // Print TVOC value
        Serial.println(ccs_2.getTVOC());
      }
    }
  }
}

void setup() 
{
  Serial.begin(115200);
  Serial.println("CCS811 Double Test");

  // First CSS Sensor:
  // (1) Enable I2C
  Wire.begin(); 

  // Second CSS Sensor:
  // (1) Enable I2C since the two sensors have the same address
  I2C_CCS_2.begin(I2C_SDA_2, I2C_SCL_2, 400);

  // Create a task for the first sensor
  xTaskCreatePinnedToCore(
    CCS_1_Task, // Task function
    "CCS_1_Task", // Task name
    10000, // Stack size
    NULL, // Parameters
    2, // Priority
    &CCS_1_Task_Handle, // Task handle
    0 // Core
  );

  // Create a task for the second sensor
  xTaskCreatePinnedToCore(
    CCS_2_Task, // Task function
    "CCS_2_Task", // Task name
    10000, // Stack size
    NULL, // Parameters
    2, // Priority
    &CCS_2_Task_Handle, // Task handle
    1 // Core
  );
}

void loop() {}