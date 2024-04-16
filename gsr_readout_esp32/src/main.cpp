#include "Arduino.h"
#include "stdio.h"     

//----------------------------------------------------------------------------------

const int GSR = 25;

int sensorValue = 0;
int gsr_average = 0;

float Resistance;
float Conductivity;

float reference_res=100;

//----------------------------------------------------------------------------------

void setup()
{
  Serial.begin(115200);
}

//----------------------------------------------------------------------------------

void loop()
{
  long sum = 0;

  for(int i = 0; i < 10; i++)           
  {
    sensorValue = analogRead(GSR);
    sum += sensorValue;
    delay(5);
  }

  gsr_average = sum / 10;  
  Serial.println(gsr_average);

  delay(1000);
}