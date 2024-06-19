//------------------------------------------------------------------------------------------------

// Readout Code for Seeed GSR Sensor Connected to an ESP32.
// The code reads the GSR sensor value and prints it to the serial monitor. This is achived by 
// taking the average of 10 sensor readings every second and doing an analog read every 100ms.

//------------------------------------------------------------------------------------------------

#include "Arduino.h"
#include "stdio.h"     

//------------------------------------------------------------------------------------------------

const int GSR_PIN = 25;

//------------------------------------------------------------------------------------------------

int sensor_value = 0;
int gsr_average = 0;

//------------------------------------------------------------------------------------------------

uint8_t LOOP_LIMIT = 10;
uint8_t LOOP_DELAY = 100;

//------------------------------------------------------------------------------------------------

void setup()
{
  Serial.begin(115200);
}

//------------------------------------------------------------------------------------------------

void loop()
{
  long sum = 0;
  uint8_t count = 0;

  if (count < LOOP_LIMIT)
  {
    sensor_value = analogRead(GSR_PIN);
    sum += sensor_value;
    count++;
  }
  else
  {
    gsr_average = sum / LOOP_LIMIT;
    Serial.println(gsr_average);
    count = 0;
    sum = 0;
  }

  delay(LOOP_DELAY);
}

//------------------------------------------------------------------------------------------------
